#ifndef __SERVO_H__
#define __SERVO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#include <stdint.h>

typedef enum
{
  SERVO_CHANNEL_1 = 0U,
  SERVO_CHANNEL_2,
  SERVO_CHANNEL_COUNT
} Servo_Channel_t;

#define SERVO_MIN_ANGLE_DEG       0U
#define SERVO_MAX_ANGLE_DEG       180U
#define SERVO_CENTER_ANGLE_DEG    90U
#define SERVO_STEP_ANGLE_DEG      1U
#define SERVO_ANGLE_SCALE         10U
#define SERVO_MIN_ANGLE_X10       (SERVO_MIN_ANGLE_DEG * SERVO_ANGLE_SCALE)
#define SERVO_MAX_ANGLE_X10       (SERVO_MAX_ANGLE_DEG * SERVO_ANGLE_SCALE)
#define SERVO_CENTER_ANGLE_X10    (SERVO_CENTER_ANGLE_DEG * SERVO_ANGLE_SCALE)

void Servo_Init(void);
void Servo_SetChannelAngle(Servo_Channel_t channel, uint8_t angle);
void Servo_SetChannelAngleX10(Servo_Channel_t channel, uint16_t angle_x10);
void Servo_CenterChannel(Servo_Channel_t channel);
uint8_t Servo_GetChannelAngle(Servo_Channel_t channel);
uint16_t Servo_GetChannelAngleX10(Servo_Channel_t channel);
void Servo_SetAngle(uint8_t angle);
void Servo_SetAngleX10(uint16_t angle_x10);
void Servo_MoveUp(void);
void Servo_MoveDown(void);
void Servo_Center(void);
uint8_t Servo_GetAngle(void);
uint16_t Servo_GetAngleX10(void);

#ifdef __cplusplus
}
#endif

#endif /* __SERVO_H__ */
