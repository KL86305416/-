#ifndef __VISION_PROTOCOL_H__
#define __VISION_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define UART_PROTOCOL_VERSION   0x01U
#define UART_PROTOCOL_SOF1      0xAAU
#define UART_PROTOCOL_SOF2      0x55U
#define UART_PROTOCOL_SRC_F407  0x01U
#define UART_PROTOCOL_SRC_OPENMV 0x10U
#define UART_PROTOCOL_SRC_F103  0x20U

typedef enum
{
  VISION_MODE_IDLE = 0x00U,
  VISION_MODE_COLOR_BLOB = 0x01U,
  VISION_MODE_APRILTAG = 0x02U,
  VISION_MODE_SIGN_CLASSIFY = 0x03U,
  VISION_MODE_LINE_AUX = 0x04U,
  VISION_MODE_CUSTOM = 0x05U
} VisionMode;

typedef enum
{
  VISION_LABEL_NONE = 0x00U,
  VISION_LABEL_LEFT_MARK = 0x01U,
  VISION_LABEL_RIGHT_MARK = 0x02U,
  VISION_LABEL_STOP_MARK = 0x03U,
  VISION_LABEL_TURN_BACK = 0x04U,
  VISION_LABEL_RED_TARGET = 0x10U,
  VISION_LABEL_GREEN_TARGET = 0x11U,
  VISION_LABEL_BLUE_TARGET = 0x12U,
  VISION_LABEL_APRILTAG_GENERIC = 0x20U,
  VISION_LABEL_CUSTOM_0 = 0x30U
} VisionLabel;

typedef enum
{
  VISION_MSG_PING = 0x01U,
  VISION_MSG_MODE_SET = 0x02U,
  VISION_MSG_RUN_CTRL = 0x03U,
  VISION_MSG_PARAM_SET = 0x04U,
  VISION_MSG_REQ_STATUS = 0x05U,
  VISION_MSG_REQ_SNAPSHOT = 0x06U,
  VISION_MSG_HELLO = 0x10U,
  VISION_MSG_STATUS = 0x11U,
  VISION_MSG_RESULT = 0x12U,
  VISION_MSG_DEBUG_TEXT = 0x13U,
  VISION_MSG_ERROR_REPORT = 0x14U,
  VISION_MSG_ACK = 0x7FU
} VisionMessageType;

typedef struct
{
  uint8_t src;
  uint8_t type;
  uint8_t seq;
  uint16_t payload_len;
  const uint8_t *payload;
} UartProtocolFrame;

typedef struct
{
  uint8_t dev_id;
  uint8_t fw_major;
  uint8_t fw_minor;
  uint16_t capability_mask;
} VisionHelloPayload;

typedef struct
{
  uint8_t state;
  uint8_t mode;
  uint16_t fps_x10;
  uint16_t latency_ms;
  uint16_t err_flags;
  uint16_t uptime_s;
} VisionStatusPayload;

typedef struct
{
  uint8_t mode;
  uint8_t target_count;
  uint8_t label;
  uint8_t confidence;
  int16_t cx;
  int16_t cy;
  uint16_t w;
  uint16_t h;
  int16_t angle_x10;
  uint16_t distance_x10;
  uint16_t flags;
  uint16_t frame_id;
} VisionResultPayload;

typedef struct
{
  uint8_t online;
  uint8_t ready;
  uint8_t last_type;
  uint8_t last_seq;
  uint32_t last_rx_ms;
  VisionHelloPayload hello;
  VisionStatusPayload status;
  VisionResultPayload result;
} VisionModuleState;

void VisionModule_Init(VisionModuleState *state);
uint8_t VisionModule_HandleFrame(VisionModuleState *state,
                                 const UartProtocolFrame *frame,
                                 uint32_t tick_ms);
uint8_t VisionModule_IsOnline(const VisionModuleState *state, uint32_t now_ms, uint32_t timeout_ms);

uint16_t VisionProtocol_BuildFrame(uint8_t src,
                                   uint8_t type,
                                   uint8_t seq,
                                   const uint8_t *payload,
                                   uint16_t payload_len,
                                   uint8_t *buffer,
                                   uint16_t capacity);
uint16_t VisionProtocol_BuildAckFrame(uint8_t seq,
                                      uint8_t acked_type,
                                      uint8_t result,
                                      uint8_t *buffer,
                                      uint16_t capacity);
uint16_t VisionProtocol_BuildModeSetFrame(uint8_t seq,
                                          uint8_t mode,
                                          uint8_t *buffer,
                                          uint16_t capacity);
uint16_t VisionProtocol_BuildRunCtrlFrame(uint8_t seq,
                                          uint8_t run,
                                          uint8_t *buffer,
                                          uint16_t capacity);
uint8_t VisionProtocol_ParseFrame(const uint8_t *data, uint16_t length, UartProtocolFrame *frame);
uint16_t VisionProtocol_Crc16CcittFalse(const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __VISION_PROTOCOL_H__ */
