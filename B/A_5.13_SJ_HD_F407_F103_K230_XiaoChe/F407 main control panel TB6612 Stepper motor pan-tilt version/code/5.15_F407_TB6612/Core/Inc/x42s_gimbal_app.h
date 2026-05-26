#ifndef __X42S_GIMBAL_APP_H__
#define __X42S_GIMBAL_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

HAL_StatusTypeDef X42sGimbalApp_Init(void);
void X42sGimbalApp_Poll(void);

#ifdef __cplusplus
}
#endif

#endif /* __X42S_GIMBAL_APP_H__ */
