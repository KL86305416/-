#ifndef __OPENMV_VISION_H__
#define __OPENMV_VISION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 11-byte protocol shared with the OpenMV side:
 *   [0]  0xA5
 *   [1]  0x5A
 *   [2-3] x_offset (int16 little-endian)
 *   [4-5] y_offset (int16 little-endian)
 *   [6-7] width    (uint16 little-endian)
 *   [8]  flags
 *   [9]  sequence
 *   [10] crc8 over bytes [2..9] (poly 0x07, init 0x00)
 */
#define OPENMV_VISION_SOF1            0xA5U
#define OPENMV_VISION_SOF2            0x5AU
#define OPENMV_VISION_FRAME_LEN       11U
#define OPENMV_VISION_PAYLOAD_LEN     8U
#define OPENMV_VISION_OFFLINE_TIMEOUT 300U

#define OPENMV_VISION_FLAG_VALID       0x01U
#define OPENMV_VISION_FLAG_RECT_FOUND  0x02U
#define OPENMV_VISION_FLAG_RECT_GOOD   0x04U

typedef struct
{
  int16_t  x_offset;     /* rect center X relative to image center        */
  int16_t  y_offset;     /* rect center Y relative to image center        */
  uint16_t width;        /* rect bounding-box width in telemetry units    */
  uint8_t  flags;        /* OPENMV_VISION_FLAG_*                          */
  uint8_t  sequence;     /* rolling 8-bit sequence from the sender        */
  uint32_t timestamp_ms; /* HAL_GetTick() at the moment the frame closed  */
} OpenMVVisionFrame;

typedef struct
{
  OpenMVVisionFrame latest;
  uint32_t          rx_frames;
  uint32_t          rx_crc_errors;
  uint32_t          rx_sync_drops;
  uint8_t           online;
} OpenMVVisionStats;

HAL_StatusTypeDef OpenMVVision_Init(void);
void              OpenMVVision_Poll(uint32_t now_ms);
uint8_t           OpenMVVision_HasFix(void);
uint8_t           OpenMVVision_IsOnline(void);
const OpenMVVisionFrame *OpenMVVision_GetLatest(void);
const OpenMVVisionStats *OpenMVVision_GetStats(void);

/* Called by HAL_UART_RxCpltCallback and the error handler. */
void OpenMVVision_OnRxByte(void);
void OpenMVVision_OnRxError(void);

#ifdef __cplusplus
}
#endif

#endif /* __OPENMV_VISION_H__ */
