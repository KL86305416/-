#include "x42s_gimbal_app.h"

#include "oled_display.h"
#include "x42s_can_bus.h"

#include <stdio.h>
#include <string.h>

#define X42S_APP_STARTUP_GAP_MS     80U
#define X42S_APP_TELEMETRY_GAP_MS   100U
#define X42S_APP_OLED_REFRESH_MS    150U

typedef enum
{
  X42S_APP_BOOT_READ_VERSION_M1 = 0,
  X42S_APP_BOOT_READ_VERSION_M2,
  X42S_APP_BOOT_ENABLE_M1,
  X42S_APP_BOOT_ENABLE_M2,
  X42S_APP_BOOT_DONE
} X42sAppBootPhase;

typedef struct
{
  uint8_t oled_ready;
  X42sAppBootPhase boot_phase;
  uint8_t telemetry_slot;
  uint32_t last_boot_tick;
  uint32_t last_probe_tick;
  uint32_t last_oled_tick;
} X42sGimbalAppState;

static X42sGimbalAppState g_app;

static void X42sGimbalApp_FormatMotorNode(char *line,
                                          size_t line_len,
                                          const char *name,
                                          const X42sMotorNode *node)
{
  if ((line == NULL) || (line_len == 0U) || (name == NULL) || (node == NULL))
  {
    return;
  }

  (void)snprintf(line,
                 line_len,
                 "%s C%u A%02u %s",
                 name,
                 node->bus_id,
                 node->motor_addr,
                 (node->online != 0U) ? "ON" : "OFF");
}

static void X42sGimbalApp_UpdateOled(uint32_t now_ms)
{
  char line[22];
  const X42sMotorNode *motor_1;
  const X42sMotorNode *motor_2;

  if (g_app.oled_ready == 0U)
  {
    return;
  }

  motor_1 = X42sCanBus_GetMotorNode(X42S_MOTOR_1);
  motor_2 = X42sCanBus_GetMotorNode(X42S_MOTOR_2);

  OledDisplay_Clear();

  (void)snprintf(line, sizeof(line), "X42S CAN %s", X42sCanBus_GetProtocolName());
  OledDisplay_PrintLine(0U, line);

  X42sGimbalApp_FormatMotorNode(line, sizeof(line), "M1", motor_1);
  OledDisplay_PrintLine(1U, line);

  X42sGimbalApp_FormatMotorNode(line, sizeof(line), "M2", motor_2);
  OledDisplay_PrintLine(2U, line);

  if (motor_1 != NULL)
  {
    (void)snprintf(line,
                   sizeof(line),
                   "M1 T%02X R%02X L%02u",
                   motor_1->last_tx_cmd,
                   motor_1->last_rx_cmd,
                   motor_1->last_rx_stream_len);
    OledDisplay_PrintLine(3U, line);

    (void)snprintf(line, sizeof(line), "M1 E%08lX", (unsigned long)motor_1->last_error);
    OledDisplay_PrintLine(4U, line);
  }

  if (motor_2 != NULL)
  {
    (void)snprintf(line,
                   sizeof(line),
                   "M2 T%02X R%02X L%02u",
                   motor_2->last_tx_cmd,
                   motor_2->last_rx_cmd,
                   motor_2->last_rx_stream_len);
    OledDisplay_PrintLine(5U, line);

    (void)snprintf(line, sizeof(line), "M2 E%08lX", (unsigned long)motor_2->last_error);
    OledDisplay_PrintLine(6U, line);
  }

  (void)snprintf(line, sizeof(line), "UP %08lu", (unsigned long)now_ms);
  OledDisplay_PrintLine(7U, line);
  OledDisplay_Update();
}

static void X42sGimbalApp_ServiceBoot(uint32_t now_ms)
{
  HAL_StatusTypeDef status = HAL_OK;

  if ((now_ms - g_app.last_boot_tick) < X42S_APP_STARTUP_GAP_MS)
  {
    return;
  }

  switch (g_app.boot_phase)
  {
    case X42S_APP_BOOT_READ_VERSION_M1:
      status = X42sCanBus_ReadVersion(X42S_MOTOR_1);
      break;

    case X42S_APP_BOOT_READ_VERSION_M2:
      status = X42sCanBus_ReadVersion(X42S_MOTOR_2);
      break;

    case X42S_APP_BOOT_ENABLE_M1:
      status = X42sCanBus_Enable(X42S_MOTOR_1, 1U, 0U);
      break;

    case X42S_APP_BOOT_ENABLE_M2:
      status = X42sCanBus_Enable(X42S_MOTOR_2, 1U, 0U);
      break;

    case X42S_APP_BOOT_DONE:
    default:
      return;
  }

  g_app.last_boot_tick = now_ms;
  if (status == HAL_OK)
  {
    g_app.boot_phase++;
  }
}

static void X42sGimbalApp_ServiceTelemetry(uint32_t now_ms)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (g_app.boot_phase != X42S_APP_BOOT_DONE)
  {
    return;
  }

  if ((now_ms - g_app.last_probe_tick) < X42S_APP_TELEMETRY_GAP_MS)
  {
    return;
  }

  switch (g_app.telemetry_slot)
  {
    case 0U:
      status = X42sCanBus_ReadStatus(X42S_MOTOR_1);
      break;

    case 1U:
      status = X42sCanBus_ReadStatus(X42S_MOTOR_2);
      break;

    case 2U:
      status = X42sCanBus_ReadPosition(X42S_MOTOR_1);
      break;

    case 3U:
      status = X42sCanBus_ReadPosition(X42S_MOTOR_2);
      break;

    case 4U:
      status = X42sCanBus_ReadSpeed(X42S_MOTOR_1);
      break;

    case 5U:
      status = X42sCanBus_ReadSpeed(X42S_MOTOR_2);
      break;

    case 6U:
      status = X42sCanBus_ReadHomingStatus(X42S_MOTOR_1);
      break;

    case 7U:
    default:
      status = X42sCanBus_ReadHomingStatus(X42S_MOTOR_2);
      break;
  }

  g_app.last_probe_tick = now_ms;
  if (status == HAL_OK)
  {
    g_app.telemetry_slot = (uint8_t)((g_app.telemetry_slot + 1U) % 8U);
  }
}

HAL_StatusTypeDef X42sGimbalApp_Init(void)
{
  (void)memset(&g_app, 0, sizeof(g_app));

  g_app.oled_ready = OledDisplay_Init();
  g_app.boot_phase = X42S_APP_BOOT_READ_VERSION_M1;

  return X42sCanBus_Init();
}

void X42sGimbalApp_Poll(void)
{
  uint32_t now_ms = HAL_GetTick();

  X42sCanBus_Poll(now_ms);
  X42sGimbalApp_ServiceBoot(now_ms);
  X42sGimbalApp_ServiceTelemetry(now_ms);

  if ((now_ms - g_app.last_oled_tick) >= X42S_APP_OLED_REFRESH_MS)
  {
    X42sGimbalApp_UpdateOled(now_ms);
    g_app.last_oled_tick = now_ms;
  }
}
