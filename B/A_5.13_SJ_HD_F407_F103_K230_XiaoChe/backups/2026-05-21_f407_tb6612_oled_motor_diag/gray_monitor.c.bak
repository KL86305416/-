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

void GrayMonitor_Poll(const GrayModuleState *state)
{
  char line[24];
  uint32_t now_ms;
  uint32_t age_ms;
  uint8_t online;

  if ((state == NULL) || (OledDisplay_IsReady() == 0U))
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
  OledDisplay_PrintLine(0U, "F103 GRAY RX");

  (void)snprintf(line, sizeof(line), "Online:%u Age:%lu", online, age_ms);
  OledDisplay_PrintLine(1U, line);

  (void)snprintf(line, sizeof(line), "Turn:%d", (int)state->runtime.turn);
  OledDisplay_PrintLine(2U, line);

  (void)snprintf(line, sizeof(line), "Scene:%s", GrayMonitor_SceneToText(state->runtime.scene));
  OledDisplay_PrintLine(3U, line);

  (void)snprintf(line, sizeof(line), "Flags:0x%02X", state->runtime.flags);
  OledDisplay_PrintLine(4U, line);

  (void)snprintf(line, sizeof(line), "Ln:%u Ct:%u",
                 (state->runtime.flags & GRAY_MODULE_RUNTIME_FLAG_LINE_FOUND) != 0U,
                 (state->runtime.flags & GRAY_MODULE_RUNTIME_FLAG_CENTER_ON_LINE) != 0U);
  OledDisplay_PrintLine(5U, line);

  (void)snprintf(line, sizeof(line), "L:%u R:%u W:%u",
                 (state->runtime.flags & GRAY_MODULE_RUNTIME_FLAG_LEFT_FEATURE) != 0U,
                 (state->runtime.flags & GRAY_MODULE_RUNTIME_FLAG_RIGHT_FEATURE) != 0U,
                 (state->runtime.flags & GRAY_MODULE_RUNTIME_FLAG_WIDE_BLACK) != 0U);
  OledDisplay_PrintLine(6U, line);

  (void)snprintf(line, sizeof(line), "Un:%u Ft:%u V:%u",
                 (state->runtime.flags & GRAY_MODULE_RUNTIME_FLAG_UNSTABLE) != 0U,
                 (state->runtime.flags & GRAY_MODULE_RUNTIME_FLAG_SENSOR_FAULT) != 0U,
                 (state->runtime.flags & GRAY_MODULE_RUNTIME_FLAG_VALID) != 0U);
  OledDisplay_PrintLine(7U, line);

  OledDisplay_Update();
}
