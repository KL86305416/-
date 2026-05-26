#include "gimbal_tracker.h"

#include "openmv_vision.h"

#include <string.h>

/* ===========================================================================
 *                          TUNABLE PARAMETERS
 *
 * Re-tune these to match the physical assembly. The most common changes:
 *   - GIMBAL_YAW_MOTOR / GIMBAL_PITCH_MOTOR if the CAN1/CAN2 connectors are
 *     swapped on your gimbal,
 *   - GIMBAL_INVERT_YAW / GIMBAL_INVERT_PITCH if a positive error makes the
 *     gimbal move AWAY from the target instead of toward it (the loop will
 *     diverge — slap the corresponding INVERT macro to 1U),
 *   - GIMBAL_KP_*_NUM / GIMBAL_KP_*_DEN to set "how many RPM per pixel of
 *     error" — too high and the rig hunts/oscillates, too low and it crawls.
 * =========================================================================== */

/* Axis to physical motor mapping. */
#define GIMBAL_YAW_MOTOR              X42S_MOTOR_1
#define GIMBAL_PITCH_MOTOR            X42S_MOTOR_2

/* If a positive error commands the wrong direction, flip these to 1U. */
#define GIMBAL_INVERT_YAW             0U
#define GIMBAL_INVERT_PITCH           0U

/* Proportional gain expressed as RPM per pixel of offset.
 *   v_target_rpm = |error_px| * NUM / DEN
 * The OpenMV side scales offsets to the telemetry frame (128x160 by default),
 * so x errors are bounded by ~64 and y errors by ~80 in normal operation. */
#define GIMBAL_KP_YAW_NUM             3
#define GIMBAL_KP_YAW_DEN             1
#define GIMBAL_KP_PITCH_NUM           3
#define GIMBAL_KP_PITCH_DEN           1

/* Output limits (RPM) and ramp.  V_MIN is the minimum non-zero command we
 * ever send to overcome stepper static friction; below the deadband we send
 * exactly zero. */
#define GIMBAL_V_MAX_RPM              200U
#define GIMBAL_V_MIN_RPM              15U
#define GIMBAL_RAMP                   200U

/* Error magnitudes below the deadband are clamped to zero RPM — keeps the
 * rig still when the target is already centred. */
#define GIMBAL_DEADBAND_X             3
#define GIMBAL_DEADBAND_Y             3

/* Control loop period — 50 ms gives a 20 Hz update rate which matches what
 * the OpenMV side produces. */
#define GIMBAL_UPDATE_PERIOD_MS       50U

/* ========================================================================= */

static GimbalTrackerSnapshot g_snap;

/* Last-sent cache so identical commands do not re-flood the CAN bus. */
static uint8_t  g_yaw_last_dir   = 0xFFU;
static uint16_t g_yaw_last_vel   = 0xFFFFU;
static uint8_t  g_pitch_last_dir = 0xFFU;
static uint16_t g_pitch_last_vel = 0xFFFFU;

static int16_t GimbalTracker_Abs16(int16_t value)
{
  return (value < 0) ? (int16_t)(-value) : value;
}

static uint16_t GimbalTracker_OffsetToRpm(int16_t error,
                                          int16_t deadband,
                                          int      kp_num,
                                          int      kp_den)
{
  int16_t  abs_error;
  int32_t  velocity;

  abs_error = GimbalTracker_Abs16(error);
  if (abs_error <= deadband)
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

static HAL_StatusTypeDef GimbalTracker_DriveAxis(X42sMotorId motor,
                                                 uint8_t   direction,
                                                 uint16_t  velocity_rpm,
                                                 uint8_t  *cached_dir,
                                                 uint16_t *cached_vel)
{
  HAL_StatusTypeDef status;

  /* Send only when the command actually changes to keep CAN load low. */
  if ((*cached_dir == direction) && (*cached_vel == velocity_rpm))
  {
    return HAL_OK;
  }

  status = X42sCanBus_SetVelocity(motor,
                                  direction,
                                  velocity_rpm,
                                  (uint16_t)GIMBAL_RAMP,
                                  0U);
  if (status == HAL_OK)
  {
    *cached_dir = direction;
    *cached_vel = velocity_rpm;
    g_snap.commands_sent++;
  }
  return status;
}

static void GimbalTracker_StopBoth(void)
{
  (void)GimbalTracker_DriveAxis(GIMBAL_YAW_MOTOR, 0U, 0U,
                                &g_yaw_last_dir, &g_yaw_last_vel);
  (void)GimbalTracker_DriveAxis(GIMBAL_PITCH_MOTOR, 0U, 0U,
                                &g_pitch_last_dir, &g_pitch_last_vel);
  g_snap.last_v_yaw_rpm   = 0U;
  g_snap.last_v_pitch_rpm = 0U;
  g_snap.tracking         = 0U;
}

HAL_StatusTypeDef GimbalTracker_Init(void)
{
  (void)memset(&g_snap, 0, sizeof(g_snap));
  g_snap.enabled = 1U;

  g_yaw_last_dir   = 0xFFU;
  g_yaw_last_vel   = 0xFFFFU;
  g_pitch_last_dir = 0xFFU;
  g_pitch_last_vel = 0xFFFFU;

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
  const X42sMotorNode     *yaw_node;
  const X42sMotorNode     *pitch_node;
  const OpenMVVisionFrame *vision;
  uint8_t  motors_online;
  int16_t  ex;
  int16_t  ey;
  uint16_t v_yaw;
  uint16_t v_pitch;
  uint8_t  dir_yaw;
  uint8_t  dir_pitch;

  if ((now_ms - g_snap.last_update_ms) < (uint32_t)GIMBAL_UPDATE_PERIOD_MS)
  {
    return;
  }
  g_snap.last_update_ms = now_ms;

  if (g_snap.enabled == 0U)
  {
    return;
  }

  yaw_node    = X42sCanBus_GetMotorNode(GIMBAL_YAW_MOTOR);
  pitch_node  = X42sCanBus_GetMotorNode(GIMBAL_PITCH_MOTOR);
  motors_online = ((yaw_node != NULL)
                   && (pitch_node != NULL)
                   && (yaw_node->online != 0U)
                   && (pitch_node->online != 0U)) ? 1U : 0U;
  g_snap.motors_online = motors_online;

  if (motors_online == 0U)
  {
    /* Without a confirmed motor we do not blast SetVelocity onto the bus.
     * The application-layer boot sequence will enable the motors first. */
    g_snap.tracking = 0U;
    return;
  }

  vision = OpenMVVision_GetLatest();
  g_snap.vision_online = OpenMVVision_IsOnline();

  if ((g_snap.vision_online == 0U)
      || (OpenMVVision_HasFix() == 0U)
      || (vision == NULL))
  {
    GimbalTracker_StopBoth();
    g_snap.locked = 0U;
    return;
  }

  ex = vision->x_offset;
  ey = vision->y_offset;
  g_snap.last_ex = ex;
  g_snap.last_ey = ey;

  v_yaw = GimbalTracker_OffsetToRpm(ex,
                                    (int16_t)GIMBAL_DEADBAND_X,
                                    GIMBAL_KP_YAW_NUM,
                                    GIMBAL_KP_YAW_DEN);
  v_pitch = GimbalTracker_OffsetToRpm(ey,
                                      (int16_t)GIMBAL_DEADBAND_Y,
                                      GIMBAL_KP_PITCH_NUM,
                                      GIMBAL_KP_PITCH_DEN);

  dir_yaw   = (ex > 0) ? 0U : 1U;
  dir_pitch = (ey > 0) ? 0U : 1U;
#if GIMBAL_INVERT_YAW
  dir_yaw = (dir_yaw != 0U) ? 0U : 1U;
#endif
#if GIMBAL_INVERT_PITCH
  dir_pitch = (dir_pitch != 0U) ? 0U : 1U;
#endif

  (void)GimbalTracker_DriveAxis(GIMBAL_YAW_MOTOR,
                                dir_yaw,
                                v_yaw,
                                &g_yaw_last_dir,
                                &g_yaw_last_vel);
  (void)GimbalTracker_DriveAxis(GIMBAL_PITCH_MOTOR,
                                dir_pitch,
                                v_pitch,
                                &g_pitch_last_dir,
                                &g_pitch_last_vel);

  g_snap.last_v_yaw_rpm   = v_yaw;
  g_snap.last_v_pitch_rpm = v_pitch;
  g_snap.last_dir_yaw     = dir_yaw;
  g_snap.last_dir_pitch   = dir_pitch;
  g_snap.tracking         = ((v_yaw > 0U) || (v_pitch > 0U)) ? 1U : 0U;
  g_snap.locked = ((GimbalTracker_Abs16(ex) <= (int16_t)GIMBAL_DEADBAND_X)
                   && (GimbalTracker_Abs16(ey) <= (int16_t)GIMBAL_DEADBAND_Y))
                  ? 1U : 0U;
}

const GimbalTrackerSnapshot *GimbalTracker_GetSnapshot(void)
{
  return &g_snap;
}
