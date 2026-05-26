#include "tracker_monitor.h"

#include "gimbal_tracker.h"
#include "oled_display.h"
#include "openmv_vision.h"

#include <stdio.h>

#define TRACKER_MONITOR_REFRESH_MS 150U

static uint8_t  g_oled_ready = 0U;
static uint32_t g_last_refresh_ms = 0U;

void TrackerMonitor_Init(void)
{
  g_oled_ready = OledDisplay_Init();
  g_last_refresh_ms = 0U;

  if (g_oled_ready != 0U)
  {
    OledDisplay_Clear();
    OledDisplay_PrintLine(0U, "UART TRACKER");
    OledDisplay_PrintLine(2U, "OLED READY");
    OledDisplay_Update();
  }
}

void TrackerMonitor_Poll(uint32_t now_ms)
{
  char line[24];
  const GimbalTrackerSnapshot *snap;
  const OpenMVVisionStats *vision_stats;

  if (g_oled_ready == 0U)
  {
    return;
  }

  if ((now_ms - g_last_refresh_ms) < TRACKER_MONITOR_REFRESH_MS)
  {
    return;
  }

  g_last_refresh_ms = now_ms;
  snap = GimbalTracker_GetSnapshot();
  vision_stats = OpenMVVision_GetStats();
  if ((snap == NULL) || (vision_stats == NULL))
  {
    return;
  }

  OledDisplay_Clear();
  OledDisplay_PrintLine(0U, "UART TRACKER");

  (void)snprintf(line, sizeof(line), "RD:%u OL:%u FX:%u",
                 snap->motors_ready,
                 snap->vision_online,
                 snap->target_valid);
  OledDisplay_PrintLine(1U, line);

  (void)snprintf(line, sizeof(line), "TR:%u LK:%u RX:%lu",
                 snap->tracking,
                 snap->locked,
                 (unsigned long)vision_stats->rx_frames);
  OledDisplay_PrintLine(2U, line);

  (void)snprintf(line, sizeof(line), "X:%d Y:%d",
                 (int)snap->last_ex,
                 (int)snap->last_ey);
  OledDisplay_PrintLine(3U, line);

  (void)snprintf(line, sizeof(line), "W:%u F:%02X",
                 snap->last_width,
                 snap->last_flags);
  OledDisplay_PrintLine(4U, line);

  (void)snprintf(line, sizeof(line), "PAN:%u D:%u",
                 snap->last_v_pan_rpm,
                 snap->last_dir_pan);
  OledDisplay_PrintLine(5U, line);

  (void)snprintf(line, sizeof(line), "TIL:%u D:%u",
                 snap->last_v_tilt_rpm,
                 snap->last_dir_tilt);
  OledDisplay_PrintLine(6U, line);

  (void)snprintf(line, sizeof(line), "CRC:%lu SY:%lu",
                 (unsigned long)vision_stats->rx_crc_errors,
                 (unsigned long)vision_stats->rx_sync_drops);
  OledDisplay_PrintLine(7U, line);

  OledDisplay_Update();
}
