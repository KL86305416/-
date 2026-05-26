#ifndef __GIMBAL_TRACKER_H__
#define __GIMBAL_TRACKER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "x42s_can_bus.h"

typedef struct
{
  uint8_t  enabled;          /* tracker armed by application                 */
  uint8_t  vision_online;    /* OpenMV uplink alive within timeout           */
  uint8_t  motors_online;    /* both yaw and pitch motors heard back on CAN  */
  uint8_t  tracking;         /* this cycle is driving at least one axis      */
  uint8_t  locked;           /* both errors within deadband                  */
  int16_t  last_ex;
  int16_t  last_ey;
  uint16_t last_v_yaw_rpm;
  uint16_t last_v_pitch_rpm;
  uint8_t  last_dir_yaw;
  uint8_t  last_dir_pitch;
  uint32_t commands_sent;
  uint32_t last_update_ms;
} GimbalTrackerSnapshot;

HAL_StatusTypeDef GimbalTracker_Init(void);
void              GimbalTracker_Poll(uint32_t now_ms);
void              GimbalTracker_SetEnabled(uint8_t enabled);
uint8_t           GimbalTracker_IsEnabled(void);
const GimbalTrackerSnapshot *GimbalTracker_GetSnapshot(void);

#ifdef __cplusplus
}
#endif

#endif /* __GIMBAL_TRACKER_H__ */
