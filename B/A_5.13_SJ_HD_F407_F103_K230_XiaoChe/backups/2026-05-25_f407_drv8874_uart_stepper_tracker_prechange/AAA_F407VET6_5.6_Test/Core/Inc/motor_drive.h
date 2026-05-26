#ifndef __MOTOR_DRIVE_H__
#define __MOTOR_DRIVE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef enum
{
  MOTOR_DRIVE_A = 0,
  MOTOR_DRIVE_B,
  MOTOR_DRIVE_C,
  MOTOR_DRIVE_D,
  MOTOR_DRIVE_COUNT
} MotorDriveChannel;

HAL_StatusTypeDef MotorDrive_Init(void);
void MotorDrive_StopAll(void);
void MotorDrive_SetPercent(MotorDriveChannel motor, int16_t percent);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_DRIVE_H__ */
