#ifndef __OLED_DISPLAY_H__
#define __OLED_DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define OLED_DISPLAY_WIDTH      128U
#define OLED_DISPLAY_HEIGHT     64U
#define OLED_DISPLAY_PAGE_COUNT (OLED_DISPLAY_HEIGHT / 8U)

uint8_t OledDisplay_Init(void);
uint8_t OledDisplay_IsReady(void);
void OledDisplay_Clear(void);
void OledDisplay_PrintLine(uint8_t line, const char *text);
void OledDisplay_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_DISPLAY_H__ */
