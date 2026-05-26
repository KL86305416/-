#include "emm_ttl_motor.h"

#include "usart.h"

#include <string.h>

#define EMM_TTL_UART_TIMEOUT_MS  20U
#define EMM_TTL_BOOT_GAP_MS      120U
/* Both motors are on separate UART links, so keeping both at address 0x01 is
 * usually the safest default. If you have already assigned unique addresses,
 * adjust these two values to match the actual motors. */
#define EMM_TTL_PAN_ADDRESS      0x01U
#define EMM_TTL_TILT_ADDRESS     0x01U
#define EMM_TTL_DEFAULT_ACCEL    20U
#define EMM_TTL_MAX_RPM          3000U

typedef enum
{
  EMM_TTL_BOOT_ENABLE_PAN = 0,
  EMM_TTL_BOOT_ENABLE_TILT,
  EMM_TTL_BOOT_DONE
} EmmTtlBootPhase;

typedef struct
{
  UART_HandleTypeDef *huart;
  EmmTtlAxisState     state;
} EmmTtlAxisContext;

static EmmTtlAxisContext  g_axes[EMM_TTL_AXIS_COUNT];
static EmmTtlMotorBusState g_bus;

static EmmTtlAxisContext *EmmTtlMotor_GetAxisContextMutable(EmmTtlAxis axis)
{
  if ((uint32_t)axis >= (uint32_t)EMM_TTL_AXIS_COUNT)
  {
    return NULL;
  }

  return &g_axes[(uint32_t)axis];
}

static void EmmTtlMotor_InitAxis(EmmTtlAxis axis,
                                 UART_HandleTypeDef *huart,
                                 uint8_t address)
{
  EmmTtlAxisContext *context;

  context = EmmTtlMotor_GetAxisContextMutable(axis);
  if (context == NULL)
  {
    return;
  }

  (void)memset(context, 0, sizeof(*context));
  context->huart = huart;
  context->state.address = address;
  context->state.last_dir = 0xFFU;
  context->state.last_velocity_rpm = 0xFFFFU;
  context->state.last_status = HAL_OK;
}

static HAL_StatusTypeDef EmmTtlMotor_Transmit(EmmTtlAxisContext *context,
                                              const uint8_t *data,
                                              uint16_t length)
{
  HAL_StatusTypeDef status;

  if ((context == NULL) || (context->huart == NULL) || (data == NULL) || (length == 0U))
  {
    return HAL_ERROR;
  }

  status = HAL_UART_Transmit(context->huart, (uint8_t *)data, length, EMM_TTL_UART_TIMEOUT_MS);
  context->state.last_status = status;
  context->state.last_tx_ms = HAL_GetTick();
  if (status == HAL_OK)
  {
    context->state.tx_count++;
  }

  return status;
}

HAL_StatusTypeDef EmmTtlMotor_Init(void)
{
  (void)memset(&g_bus, 0, sizeof(g_bus));
  /* Current rig mapping confirmed on 2026-05-26:
   * - UART5  -> tilt / up-down axis
   * - USART6 -> pan / left-right axis */
  EmmTtlMotor_InitAxis(EMM_TTL_AXIS_PAN, &huart6, EMM_TTL_PAN_ADDRESS);
  EmmTtlMotor_InitAxis(EMM_TTL_AXIS_TILT, &huart5, EMM_TTL_TILT_ADDRESS);
  g_bus.boot_phase = (uint8_t)EMM_TTL_BOOT_ENABLE_PAN;
  g_bus.last_boot_ms = HAL_GetTick();
  return HAL_OK;
}

void EmmTtlMotor_Poll(uint32_t now_ms)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (g_bus.ready != 0U)
  {
    return;
  }

  if ((now_ms - g_bus.last_boot_ms) < EMM_TTL_BOOT_GAP_MS)
  {
    return;
  }

  switch ((EmmTtlBootPhase)g_bus.boot_phase)
  {
    case EMM_TTL_BOOT_ENABLE_PAN:
      status = EmmTtlMotor_Enable(EMM_TTL_AXIS_PAN, 1U);
      if (status == HAL_OK)
      {
        g_bus.boot_phase = (uint8_t)EMM_TTL_BOOT_ENABLE_TILT;
      }
      break;

    case EMM_TTL_BOOT_ENABLE_TILT:
      status = EmmTtlMotor_Enable(EMM_TTL_AXIS_TILT, 1U);
      if (status == HAL_OK)
      {
        g_bus.boot_phase = (uint8_t)EMM_TTL_BOOT_DONE;
        g_bus.ready = 1U;
      }
      break;

    case EMM_TTL_BOOT_DONE:
    default:
      g_bus.ready = 1U;
      break;
  }

  g_bus.last_boot_ms = now_ms;
}

HAL_StatusTypeDef EmmTtlMotor_Enable(EmmTtlAxis axis, uint8_t enable)
{
  EmmTtlAxisContext *context;
  uint8_t frame[6];
  HAL_StatusTypeDef status;

  context = EmmTtlMotor_GetAxisContextMutable(axis);
  if (context == NULL)
  {
    return HAL_ERROR;
  }

  frame[0] = context->state.address;
  frame[1] = 0xF3U;
  frame[2] = 0xABU;
  frame[3] = (enable != 0U) ? 0x01U : 0x00U;
  frame[4] = 0x00U;
  frame[5] = 0x6BU;

  status = EmmTtlMotor_Transmit(context, frame, (uint16_t)sizeof(frame));
  if (status == HAL_OK)
  {
    context->state.enabled = (enable != 0U) ? 1U : 0U;
    if (enable == 0U)
    {
      context->state.last_dir = 0xFFU;
      context->state.last_velocity_rpm = 0U;
    }
  }

  return status;
}

HAL_StatusTypeDef EmmTtlMotor_SetVelocity(EmmTtlAxis axis, uint8_t direction, uint16_t velocity_rpm)
{
  EmmTtlAxisContext *context;
  uint8_t frame[8];
  HAL_StatusTypeDef status;

  context = EmmTtlMotor_GetAxisContextMutable(axis);
  if (context == NULL)
  {
    return HAL_ERROR;
  }

  if (velocity_rpm == 0U)
  {
    return EmmTtlMotor_Stop(axis);
  }

  if (velocity_rpm > EMM_TTL_MAX_RPM)
  {
    velocity_rpm = EMM_TTL_MAX_RPM;
  }

  frame[0] = context->state.address;
  frame[1] = 0xF6U;
  frame[2] = (direction != 0U) ? 0x01U : 0x00U;
  frame[3] = (uint8_t)((velocity_rpm >> 8) & 0xFFU);
  frame[4] = (uint8_t)(velocity_rpm & 0xFFU);
  frame[5] = EMM_TTL_DEFAULT_ACCEL;
  frame[6] = 0x00U;
  frame[7] = 0x6BU;

  status = EmmTtlMotor_Transmit(context, frame, (uint16_t)sizeof(frame));
  if (status == HAL_OK)
  {
    context->state.last_dir = (direction != 0U) ? 1U : 0U;
    context->state.last_velocity_rpm = velocity_rpm;
  }

  return status;
}

HAL_StatusTypeDef EmmTtlMotor_Stop(EmmTtlAxis axis)
{
  EmmTtlAxisContext *context;
  uint8_t frame[5];
  HAL_StatusTypeDef status;

  context = EmmTtlMotor_GetAxisContextMutable(axis);
  if (context == NULL)
  {
    return HAL_ERROR;
  }

  frame[0] = context->state.address;
  frame[1] = 0xFEU;
  frame[2] = 0x98U;
  frame[3] = 0x00U;
  frame[4] = 0x6BU;

  status = EmmTtlMotor_Transmit(context, frame, (uint16_t)sizeof(frame));
  if (status == HAL_OK)
  {
    context->state.last_dir = 0U;
    context->state.last_velocity_rpm = 0U;
  }

  return status;
}

void EmmTtlMotor_StopAll(void)
{
  (void)EmmTtlMotor_Stop(EMM_TTL_AXIS_PAN);
  (void)EmmTtlMotor_Stop(EMM_TTL_AXIS_TILT);
}

uint8_t EmmTtlMotor_IsReady(void)
{
  return g_bus.ready;
}

const EmmTtlAxisState *EmmTtlMotor_GetAxisState(EmmTtlAxis axis)
{
  EmmTtlAxisContext *context;

  context = EmmTtlMotor_GetAxisContextMutable(axis);
  if (context == NULL)
  {
    return NULL;
  }

  return &context->state;
}

const EmmTtlMotorBusState *EmmTtlMotor_GetBusState(void)
{
  return &g_bus;
}
