#ifndef __GRAY_MODULE_H__
#define __GRAY_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define GRAY_MODULE_DEVICE_ID        0xF1U
#define GRAY_MODULE_PROTOCOL_VERSION 0x01U
#define GRAY_MODULE_I2C_ADDRESS      0x42U
#define GRAY_MODULE_I2C_BLOCK_LEN    10U
#define GRAY_MODULE_UART_STATUS_LEN  8U

typedef enum
{
  GRAY_MODULE_TRACK_NONE = 0x00U,
  GRAY_MODULE_TRACK_SINGLE_LINE = 0x01U,
  GRAY_MODULE_TRACK_CROSS = 0x02U,
  GRAY_MODULE_TRACK_ALL_BLACK = 0x03U,
  GRAY_MODULE_TRACK_LEFT_EDGE = 0x04U,
  GRAY_MODULE_TRACK_RIGHT_EDGE = 0x05U,
  GRAY_MODULE_TRACK_AMBIGUOUS = 0x06U
} GrayModuleTrackType;

typedef enum
{
  GRAY_MODULE_REG_DEV_ID = 0x00U,
  GRAY_MODULE_REG_PROTO_VER = 0x01U,
  GRAY_MODULE_REG_STATUS_FLAGS = 0x02U,
  GRAY_MODULE_REG_SENSOR_BITS = 0x03U,
  GRAY_MODULE_REG_TRACK_TYPE = 0x04U,
  GRAY_MODULE_REG_EDGE_COUNT = 0x05U,
  GRAY_MODULE_REG_CENTER_ERR_L = 0x06U,
  GRAY_MODULE_REG_CENTER_ERR_H = 0x07U,
  GRAY_MODULE_REG_FRAME_CNT_L = 0x08U,
  GRAY_MODULE_REG_FRAME_CNT_H = 0x09U,
  GRAY_MODULE_REG_CMD = 0x10U,
  GRAY_MODULE_REG_CMD_ARG0 = 0x11U,
  GRAY_MODULE_REG_CMD_ARG1 = 0x12U,
  GRAY_MODULE_REG_CMD_SEQ = 0x13U
} GrayModuleRegisterAddress;

typedef enum
{
  GRAY_MODULE_CMD_NOP = 0x00U,
  GRAY_MODULE_CMD_CLEAR_COUNTER = 0x01U,
  GRAY_MODULE_CMD_UART_STREAM_ON = 0x02U,
  GRAY_MODULE_CMD_UART_STREAM_OFF = 0x03U,
  GRAY_MODULE_CMD_SOFT_RESET = 0x04U
} GrayModuleCommandId;

typedef enum
{
  GRAY_MODULE_UART_TYPE_STATUS = 0x10U,
  GRAY_MODULE_UART_TYPE_DEBUG = 0x11U,
  GRAY_MODULE_UART_TYPE_HEARTBEAT = 0x12U,
  GRAY_MODULE_UART_TYPE_ACK = 0x7FU
} GrayModuleUartType;

typedef struct
{
  uint8_t status_flags;
  uint8_t sensor_bits;
  uint8_t track_type;
  uint8_t edge_count;
  int16_t center_err;
  uint16_t frame_cnt;
} GrayStatusFrame;

typedef struct
{
  uint8_t online;
  GrayStatusFrame status;
  uint32_t last_update_ms;
} GrayModuleState;

void GrayModule_Init(GrayModuleState *state);
uint8_t GrayModule_ParseI2cBlock(const uint8_t *data,
                                 uint16_t length,
                                 GrayModuleState *state,
                                 uint32_t tick_ms);
uint16_t GrayModule_BuildI2cCommand(uint8_t cmd,
                                    uint8_t arg0,
                                    uint8_t arg1,
                                    uint8_t seq,
                                    uint8_t *buffer,
                                    uint16_t capacity);
uint8_t GrayModule_ParseUartStatusPayload(const uint8_t *payload,
                                          uint16_t length,
                                          GrayModuleState *state,
                                          uint32_t tick_ms);
uint8_t GrayModule_IsOnline(const GrayModuleState *state, uint32_t now_ms, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* __GRAY_MODULE_H__ */
