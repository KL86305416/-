#ifndef __EMM_TTL_MOTOR_H__
#define __EMM_TTL_MOTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef enum
{
  EMM_TTL_AXIS_PAN = 0,
  EMM_TTL_AXIS_TILT,
  EMM_TTL_AXIS_COUNT
} EmmTtlAxis;

typedef struct
{
  uint8_t           address;
  uint8_t           enabled;
  uint8_t           last_dir;
  uint16_t          last_velocity_rpm;
  uint32_t          tx_count;
  uint32_t          last_tx_ms;
  HAL_StatusTypeDef last_status;
} EmmTtlAxisState;

typedef struct
{
  uint8_t  ready;
  uint8_t  boot_phase;
  uint32_t last_boot_ms;
} EmmTtlMotorBusState;

HAL_StatusTypeDef          EmmTtlMotor_Init(void);
void                       EmmTtlMotor_Poll(uint32_t now_ms);
HAL_StatusTypeDef          EmmTtlMotor_Enable(EmmTtlAxis axis, uint8_t enable);
HAL_StatusTypeDef          EmmTtlMotor_SetVelocity(EmmTtlAxis axis, uint8_t direction, uint16_t velocity_rpm);
HAL_StatusTypeDef          EmmTtlMotor_Stop(EmmTtlAxis axis);
void                       EmmTtlMotor_StopAll(void);
uint8_t                    EmmTtlMotor_IsReady(void);
const EmmTtlAxisState      *EmmTtlMotor_GetAxisState(EmmTtlAxis axis);
const EmmTtlMotorBusState  *EmmTtlMotor_GetBusState(void);

#ifdef __cplusplus
}
#endif

#endif /* __EMM_TTL_MOTOR_H__ */
