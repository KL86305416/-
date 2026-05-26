#include "gimbal_tracker.h"

#include "emm_ttl_motor.h"
#include "openmv_vision.h"

#include <string.h>

/* Re-tune these to match the installed mechanics:
 * - flip GIMBAL_INVERT_* if motion runs away from the target,
 * - tune KP / V_MIN / V_MAX to balance responsiveness and stability. */
#define GIMBAL_INVERT_PAN          0U
#define GIMBAL_INVERT_TILT         0U
#define GIMBAL_KP_PAN_NUM          3
#define GIMBAL_KP_PAN_DEN          1
#define GIMBAL_KP_TILT_NUM         3
#define GIMBAL_KP_TILT_DEN         1
#define GIMBAL_V_MAX_RPM           220U
#define GIMBAL_V_MIN_RPM           15U
#define GIMBAL_DEADBAND_X          3
#define GIMBAL_DEADBAND_Y          3
#define GIMBAL_UPDATE_PERIOD_MS    50U

static GimbalTrackerSnapshot g_snap;
static uint8_t  g_pan_last_dir = 0xFFU;
static uint16_t g_pan_last_vel = 0xFFFFU;
static uint8_t  g_tilt_last_dir = 0xFFU;
static uint16_t g_tilt_last_vel = 0xFFFFU;

static uint16_t GimbalTracker_Abs16(int16_t value)
{
  if (value < 0)
  {
    return (uint16_t)(-(int32_t)value);
  }

  return (uint16_t)value;
}

static uint16_t GimbalTracker_OffsetToRpm(int16_t error,
                                          int16_t deadband,
                                          int kp_num,
                                          int kp_den)
{
  uint16_t abs_error;
  int32_t velocity;

  abs_error = GimbalTracker_Abs16(error);
  if (abs_error <= (uint16_t)deadband)
  {
    return 0U;
  }

  velocity = ((int32_t)abs_error * (int32_t)kp_num) / (int32_t)kp_den;
  if (velocity > (int32_t)GIMBAL_V_MAX_RPM)
  {
    velocity = (int32_t)GIMBAL_V_MAX_RPM;
  }
  if (velocity < (int32_t)GIMBAL_V_MIN_RPM)
  {
    velocity = (int32_t)GIMBAL_V_MIN_RPM;
  }

  return (uint16_t)velocity;
}

static HAL_StatusTypeDef GimbalTracker_DriveAxis(EmmTtlAxis axis,
                                                 uint8_t direction,
                                                 uint16_t velocity_rpm,
                                                 uint8_t *cached_dir,
                                                 uint16_t *cached_vel)
{
  HAL_StatusTypeDef status;

  if ((velocity_rpm == 0U) && (*cached_vel == 0U))
  {
    return HAL_OK;
  }

  if ((*cached_dir == direction) && (*cached_vel == velocity_rpm))
  {
    return HAL_OK;
  }

  if (velocity_rpm == 0U)
  {
    status = EmmTtlMotor_Stop(axis);
  }
  else
  {
    status = EmmTtlMotor_SetVelocity(axis, direction, velocity_rpm);
  }

  if (status == HAL_OK)
  {
    *cached_dir = (velocity_rpm == 0U) ? 0U : direction;
    *cached_vel = velocity_rpm;
    g_snap.commands_sent++;
  }

  return status;
}

static void GimbalTracker_StopBoth(void)
{
  if ((g_pan_last_vel != 0U) || (g_tilt_last_vel != 0U))
  {
    g_snap.stop_events++;
  }

  if (g_snap.motors_ready != 0U)
  {
    (void)GimbalTracker_DriveAxis(EMM_TTL_AXIS_PAN, 0U, 0U,
                                  &g_pan_last_dir, &g_pan_last_vel);
    (void)GimbalTracker_DriveAxis(EMM_TTL_AXIS_TILT, 0U, 0U,
                                  &g_tilt_last_dir, &g_tilt_last_vel);
  }
  else
  {
    g_pan_last_dir = 0U;
    g_pan_last_vel = 0U;
    g_tilt_last_dir = 0U;
    g_tilt_last_vel = 0U;
  }

  g_snap.last_v_pan_rpm = 0U;
  g_snap.last_v_tilt_rpm = 0U;
  g_snap.tracking = 0U;
}

HAL_StatusTypeDef GimbalTracker_Init(void)
{
  (void)memset(&g_snap, 0, sizeof(g_snap));
  g_snap.enabled = 1U;
  g_pan_last_dir = 0xFFU;
  g_pan_last_vel = 0xFFFFU;
  g_tilt_last_dir = 0xFFU;
  g_tilt_last_vel = 0xFFFFU;
  return HAL_OK;
}

void GimbalTracker_SetEnabled(uint8_t enabled)
{
  uint8_t was_enabled = g_snap.enabled;

  g_snap.enabled = (enabled != 0U) ? 1U : 0U;
  if ((was_enabled != 0U) && (g_snap.enabled == 0U))
  {
    GimbalTracker_StopBoth();
  }
}

uint8_t GimbalTracker_IsEnabled(void)
{
  return g_snap.enabled;
}

void GimbalTracker_Poll(uint32_t now_ms)
{
  const OpenMVVisionFrame *vision;
  uint16_t v_pan;
  uint16_t v_tilt;
  uint8_t dir_pan;
  uint8_t dir_tilt;

  if ((now_ms - g_snap.last_update_ms) < GIMBAL_UPDATE_PERIOD_MS)
  {
    return;
  }

  g_snap.last_update_ms = now_ms;
  g_snap.motors_ready = EmmTtlMotor_IsReady();
  g_snap.vision_online = OpenMVVision_IsOnline();
  g_snap.target_valid = OpenMVVision_HasFix();

  if (g_snap.enabled == 0U)
  {
    return;
  }

  if (g_snap.motors_ready == 0U)
  {
    g_snap.tracking = 0U;
    g_snap.locked = 0U;
    return;
  }

  vision = OpenMVVision_GetLatest();
  if (vision != NULL)
  {
    g_snap.last_width = vision->width;
    g_snap.last_flags = vision->flags;
    g_snap.last_ex = vision->x_offset;
    g_snap.last_ey = vision->y_offset;
  }

  if ((g_snap.vision_online == 0U)
      || (g_snap.target_valid == 0U)
      || (vision == NULL))
  {
    GimbalTracker_StopBoth();
    g_snap.locked = 0U;
    return;
  }

  v_pan = GimbalTracker_OffsetToRpm(vision->x_offset,
                                    (int16_t)GIMBAL_DEADBAND_X,
                                    GIMBAL_KP_PAN_NUM,
                                    GIMBAL_KP_PAN_DEN);
  v_tilt = GimbalTracker_OffsetToRpm(vision->y_offset,
                                     (int16_t)GIMBAL_DEADBAND_Y,
                                     GIMBAL_KP_TILT_NUM,
                                     GIMBAL_KP_TILT_DEN);

  dir_pan = (vision->x_offset > 0) ? 0U : 1U;
  dir_tilt = (vision->y_offset > 0) ? 0U : 1U;
#if GIMBAL_INVERT_PAN
  dir_pan = (dir_pan != 0U) ? 0U : 1U;
#endif
#if GIMBAL_INVERT_TILT
  dir_tilt = (dir_tilt != 0U) ? 0U : 1U;
#endif

  (void)GimbalTracker_DriveAxis(EMM_TTL_AXIS_PAN,
                                dir_pan,
                                v_pan,
                                &g_pan_last_dir,
                                &g_pan_last_vel);
  (void)GimbalTracker_DriveAxis(EMM_TTL_AXIS_TILT,
                                dir_tilt,
                                v_tilt,
                                &g_tilt_last_dir,
                                &g_tilt_last_vel);

  g_snap.last_v_pan_rpm = v_pan;
  g_snap.last_v_tilt_rpm = v_tilt;
  g_snap.last_dir_pan = dir_pan;
  g_snap.last_dir_tilt = dir_tilt;
  g_snap.tracking = ((v_pan > 0U) || (v_tilt > 0U)) ? 1U : 0U;
  g_snap.locked = ((GimbalTracker_Abs16(vision->x_offset) <= (uint16_t)GIMBAL_DEADBAND_X)
                   && (GimbalTracker_Abs16(vision->y_offset) <= (uint16_t)GIMBAL_DEADBAND_Y))
                  ? 1U : 0U;
}

const GimbalTrackerSnapshot *GimbalTracker_GetSnapshot(void)
{
  return &g_snap;
}
