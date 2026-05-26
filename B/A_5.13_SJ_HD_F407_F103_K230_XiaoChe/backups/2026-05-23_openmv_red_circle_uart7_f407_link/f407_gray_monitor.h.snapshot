#ifndef __GRAY_MONITOR_H__
#define __GRAY_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "gray_module.h"

typedef struct
{
  int16_t motor_a_percent;
  int16_t motor_b_percent;
  int16_t left_encoder_delta;
  int16_t right_encoder_delta;
  int16_t vision_x_offset;
  uint8_t key_1_pressed;
  uint8_t key_2_pressed;
  uint8_t key_3_pressed;
  uint8_t key_4_pressed;
  uint8_t ain1_state;
  uint8_t ain2_state;
  uint8_t bin1_state;
  uint8_t bin2_state;
  uint8_t vision_online;
  uint8_t vision_flags;
  uint8_t vision_seq;
  uint16_t left_encoder_count;
  uint16_t right_encoder_count;
  uint16_t pwm_a_compare;
  uint16_t pwm_b_compare;
  uint16_t vision_radius;
} GrayMonitorDriveDiag;

void GrayMonitor_Init(void);
void GrayMonitor_Poll(const GrayModuleState *state, const GrayMonitorDriveDiag *drive_diag);

#ifdef __cplusplus
}
#endif

#endif /* __GRAY_MONITOR_H__ */
