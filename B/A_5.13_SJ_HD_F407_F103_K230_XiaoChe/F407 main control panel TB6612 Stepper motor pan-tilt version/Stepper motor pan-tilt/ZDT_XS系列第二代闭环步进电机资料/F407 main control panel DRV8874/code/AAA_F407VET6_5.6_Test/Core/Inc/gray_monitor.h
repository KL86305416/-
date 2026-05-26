#ifndef __GRAY_MONITOR_H__
#define __GRAY_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "gray_module.h"

void GrayMonitor_Init(void);
void GrayMonitor_Poll(const GrayModuleState *state);

#ifdef __cplusplus
}
#endif

#endif /* __GRAY_MONITOR_H__ */
