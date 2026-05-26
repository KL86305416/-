#ifndef __GIMBAL_TRACKER_H__
#define __GIMBAL_TRACKER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef struct
{
  uint8_t  enabled;
  uint8_t  vision_online;
  uint8_t  motors_ready;
  uint8_t  target_valid;
  uint8_t  tracking;
  uint8_t  locked;
  int16_t  last_ex;
  int16_t  last_ey;
  uint16_t last_width;
  uint8_t  last_flags;
  uint16_t last_v_pan_rpm;
  uint16_t last_v_tilt_rpm;
  uint8_t  last_dir_pan;
  uint8_t  last_dir_tilt;
  uint32_t commands_sent;
  uint32_t stop_events;
  uint32_t last_update_ms;
} GimbalTrackerSnapshot;

HAL_StatusTypeDef           GimbalTracker_Init(void);
void                        GimbalTracker_Poll(uint32_t now_ms);
void                        GimbalTracker_SetEnabled(uint8_t enabled);
uint8_t                     GimbalTracker_IsEnabled(void);
const GimbalTrackerSnapshot *GimbalTracker_GetSnapshot(void);

#ifdef __cplusplus
}
#endif

#endif /* __GIMBAL_TRACKER_H__ */
