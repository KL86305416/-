#ifndef __GRAY_MODULE_H__
#define __GRAY_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define GRAY_MODULE_DEVICE_ID          0xF1U
#define GRAY_MODULE_PROTOCOL_VERSION   0x01U
#define GRAY_MODULE_I2C_ADDRESS        0x42U
#define GRAY_MODULE_I2C_BLOCK_LEN      10U
#define GRAY_MODULE_UART_STATUS_LEN    8U
#define GRAY_MODULE_RUNTIME_FRAME_LEN  6U
#define GRAY_MODULE_UART_SOF1          0xAAU
#define GRAY_MODULE_UART_SOF2          0x55U

#define GRAY_MODULE_RUNTIME_FLAG_VALID          0x01U
#define GRAY_MODULE_RUNTIME_FLAG_LINE_FOUND     0x02U
#define GRAY_MODULE_RUNTIME_FLAG_LEFT_FEATURE   0x04U
#define GRAY_MODULE_RUNTIME_FLAG_RIGHT_FEATURE  0x08U
#define GRAY_MODULE_RUNTIME_FLAG_CENTER_ON_LINE 0x10U
#define GRAY_MODULE_RUNTIME_FLAG_WIDE_BLACK     0x20U
#define GRAY_MODULE_RUNTIME_FLAG_UNSTABLE       0x40U
#define GRAY_MODULE_RUNTIME_FLAG_SENSOR_FAULT   0x80U

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

typedef enum
{
  GRAY_MODULE_SCENE_NONE = 0x00U,
  GRAY_MODULE_SCENE_STRAIGHT = 0x01U,
  GRAY_MODULE_SCENE_SOFT_LEFT = 0x02U,
  GRAY_MODULE_SCENE_SOFT_RIGHT = 0x03U,
  GRAY_MODULE_SCENE_HARD_LEFT = 0x04U,
  GRAY_MODULE_SCENE_HARD_RIGHT = 0x05U,
  GRAY_MODULE_SCENE_CROSS = 0x06U,
  GRAY_MODULE_SCENE_LEFT_BRANCH = 0x07U,
  GRAY_MODULE_SCENE_RIGHT_BRANCH = 0x08U,
  GRAY_MODULE_SCENE_T_JUNCTION = 0x09U,
  GRAY_MODULE_SCENE_STOP_BAR = 0x0AU,
  GRAY_MODULE_SCENE_LINE_LOST = 0x0BU,
  GRAY_MODULE_SCENE_AMBIGUOUS = 0x0CU
} GrayModuleRuntimeScene;

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
  int8_t turn;
  uint8_t scene;
  uint8_t flags;
} GrayRuntimeFrame;

typedef struct
{
  uint8_t online;
  GrayStatusFrame status;
  GrayRuntimeFrame runtime;
  uint32_t last_update_ms;
  uint8_t rx_index;
  uint8_t rx_buffer[GRAY_MODULE_RUNTIME_FRAME_LEN];
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
uint8_t GrayModule_ParseRuntimeFrame(const uint8_t *frame,
                                     uint16_t length,
                                     GrayModuleState *state,
                                     uint32_t tick_ms);
uint8_t GrayModule_ConsumeRuntimeUartByte(GrayModuleState *state,
                                          uint8_t byte,
                                          uint32_t tick_ms);
void GrayModule_Service(GrayModuleState *state, uint32_t now_ms, uint32_t timeout_ms);
uint8_t GrayModule_IsOnline(const GrayModuleState *state, uint32_t now_ms, uint32_t timeout_ms);
uint8_t GrayModule_Crc8(const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __GRAY_MODULE_H__ */
