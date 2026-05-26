#include "gray_monitor.h"

#include "oled_display.h"

#include <stdio.h>

#define GRAY_MONITOR_REFRESH_MS 200U
#define GRAY_MONITOR_TIMEOUT_MS 100U

static uint32_t g_last_refresh_ms = 0U;

static const char *GrayMonitor_SceneToText(uint8_t scene)
{
  switch (scene)
  {
    case GRAY_MODULE_SCENE_STRAIGHT:     return "STRAIGHT";
    case GRAY_MODULE_SCENE_SOFT_LEFT:    return "SOFT_L";
    case GRAY_MODULE_SCENE_SOFT_RIGHT:   return "SOFT_R";
    case GRAY_MODULE_SCENE_HARD_LEFT:    return "HARD_L";
    case GRAY_MODULE_SCENE_HARD_RIGHT:   return "HARD_R";
    case GRAY_MODULE_SCENE_CROSS:        return "CROSS";
    case GRAY_MODULE_SCENE_LEFT_BRANCH:  return "L_BRANCH";
    case GRAY_MODULE_SCENE_RIGHT_BRANCH: return "R_BRANCH";
    case GRAY_MODULE_SCENE_T_JUNCTION:   return "T_JUNC";
    case GRAY_MODULE_SCENE_STOP_BAR:     return "STOP_BAR";
    case GRAY_MODULE_SCENE_LINE_LOST:    return "LINE_LOST";
    case GRAY_MODULE_SCENE_AMBIGUOUS:    return "AMBIG";
    default:                             return "NONE";
  }
}

void GrayMonitor_Init(void)
{
  if (OledDisplay_Init() != 0U)
  {
    OledDisplay_Clear();
    OledDisplay_PrintLine(0U, "F407 GRAY MON");
    OledDisplay_PrintLine(2U, "OLED READY");
    OledDisplay_Update();
  }

  g_last_refresh_ms = 0U;
}

void GrayMonitor_Poll(const GrayModuleState *state, const GrayMonitorDriveDiag *drive_diag)
{
  char line[24];
  uint32_t now_ms;
  uint32_t age_ms;
  uint8_t online;

  if ((state == NULL) || (drive_diag == NULL) || (OledDisplay_IsReady() == 0U))
  {
    return;
  }

  now_ms = HAL_GetTick();
  if ((now_ms - g_last_refresh_ms) < GRAY_MONITOR_REFRESH_MS)
  {
    return;
  }

  g_last_refresh_ms = now_ms;
  age_ms = now_ms - state->last_update_ms;
  online = GrayModule_IsOnline(state, now_ms, GRAY_MONITOR_TIMEOUT_MS);

  OledDisplay_Clear();
  OledDisplay_PrintLine(0U, "F103+OMV+DRV");

  (void)snprintf(line, sizeof(line), "G:%u V:%u K:%u%u%u%u",
                 online,
                 drive_diag->vision_online,
                 drive_diag->key_1_pressed,
                 drive_diag->key_2_pressed,
                 drive_diag->key_3_pressed,
                 drive_diag->key_4_pressed);
  OledDisplay_PrintLine(1U, line);

  (void)snprintf(line, sizeof(line), "T:%d S:%s",
                 (int)state->runtime.turn,
                 GrayMonitor_SceneToText(state->runtime.scene));
  OledDisplay_PrintLine(2U, line);

  (void)snprintf(line, sizeof(line), "Age:%lu VF:%02X",
                 age_ms,
                 drive_diag->vision_flags);
  OledDisplay_PrintLine(3U, line);

  (void)snprintf(line, sizeof(line), "DX:%+d R:%u Q:%u",
                 (int)drive_diag->vision_x_offset,
                 (unsigned int)drive_diag->vision_radius,
                 (unsigned int)drive_diag->vision_seq);
  OledDisplay_PrintLine(4U, line);

  (void)snprintf(line, sizeof(line), "L:%+d P:%u",
                 (int)drive_diag->motor_a_percent,
                 (unsigned int)drive_diag->pwm_a_compare);
  OledDisplay_PrintLine(5U, line);

  (void)snprintf(line, sizeof(line), "R:%+d P:%u",
                 (int)drive_diag->motor_b_percent,
                 (unsigned int)drive_diag->pwm_b_compare);
  OledDisplay_PrintLine(6U, line);

  (void)snprintf(line, sizeof(line), "IO:%u%u%u%u d:%d/%d",
                 drive_diag->ain1_state,
                 drive_diag->ain2_state,
                 drive_diag->bin1_state,
                 drive_diag->bin2_state,
                 (int)drive_diag->left_encoder_delta,
                 (int)drive_diag->right_encoder_delta);
  OledDisplay_PrintLine(7U, line);

  OledDisplay_Update();
}
