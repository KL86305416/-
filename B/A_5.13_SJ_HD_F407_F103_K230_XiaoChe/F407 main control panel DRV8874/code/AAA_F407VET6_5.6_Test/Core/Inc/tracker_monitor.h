#ifndef __TRACKER_MONITOR_H__
#define __TRACKER_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void TrackerMonitor_Init(void);
void TrackerMonitor_Poll(uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* __TRACKER_MONITOR_H__ */
