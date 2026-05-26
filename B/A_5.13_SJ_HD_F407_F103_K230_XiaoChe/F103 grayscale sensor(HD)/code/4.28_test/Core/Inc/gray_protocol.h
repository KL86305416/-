#ifndef __GRAY_PROTOCOL_H__
#define __GRAY_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define GRAY_PROTOCOL_VERSION         0x01U
#define GRAY_I2C_SLAVE_ADDRESS        0x42U
#define GRAY_DEVICE_ID                0xF1U

#define GRAY_UART_SOF1                0xAAU
#define GRAY_UART_SOF2                0x55U
#define GRAY_UART_SRC_F103            0x20U
#define GRAY_UART_TYPE_STATUS         0x10U
#define GRAY_UART_TYPE_DEBUG          0x11U
#define GRAY_UART_TYPE_HEARTBEAT      0x12U
#define GRAY_UART_TYPE_ACK            0x7FU

#define GRAY_STATUS_FLAG_READY        0x01U
#define GRAY_STATUS_FLAG_DATA_VALID   0x02U
#define GRAY_STATUS_FLAG_SENSOR_FAULT 0x04U
#define GRAY_STATUS_FLAG_ALL_BLACK    0x08U
#define GRAY_STATUS_FLAG_ALL_WHITE    0x10U
#define GRAY_STATUS_FLAG_UART_STREAM  0x20U

#define GRAY_RUNTIME_FRAME_LEN        6U
#define GRAY_RUNTIME_FLAG_VALID       0x01U
#define GRAY_RUNTIME_FLAG_LINE_FOUND  0x02U
#define GRAY_RUNTIME_FLAG_LEFT_FEATURE 0x04U
#define GRAY_RUNTIME_FLAG_RIGHT_FEATURE 0x08U
#define GRAY_RUNTIME_FLAG_CENTER_ON_LINE 0x10U
#define GRAY_RUNTIME_FLAG_WIDE_BLACK  0x20U
#define GRAY_RUNTIME_FLAG_UNSTABLE    0x40U
#define GRAY_RUNTIME_FLAG_SENSOR_FAULT 0x80U

typedef enum
{
  GRAY_TRACK_NONE = 0x00U,
  GRAY_TRACK_SINGLE_LINE = 0x01U,
  GRAY_TRACK_CROSS = 0x02U,
  GRAY_TRACK_ALL_BLACK = 0x03U,
  GRAY_TRACK_LEFT_EDGE = 0x04U,
  GRAY_TRACK_RIGHT_EDGE = 0x05U,
  GRAY_TRACK_AMBIGUOUS = 0x06U
} GrayTrackType;

typedef enum
{
  GRAY_REG_DEV_ID = 0x00U,
  GRAY_REG_PROTO_VER = 0x01U,
  GRAY_REG_STATUS_FLAGS = 0x02U,
  GRAY_REG_SENSOR_BITS = 0x03U,
  GRAY_REG_TRACK_TYPE = 0x04U,
  GRAY_REG_EDGE_COUNT = 0x05U,
  GRAY_REG_CENTER_ERR_L = 0x06U,
  GRAY_REG_CENTER_ERR_H = 0x07U,
  GRAY_REG_FRAME_CNT_L = 0x08U,
  GRAY_REG_FRAME_CNT_H = 0x09U,
  GRAY_REG_CMD = 0x10U,
  GRAY_REG_CMD_ARG0 = 0x11U,
  GRAY_REG_CMD_ARG1 = 0x12U,
  GRAY_REG_CMD_SEQ = 0x13U
} GrayRegisterAddress;

typedef enum
{
  GRAY_CMD_NOP = 0x00U,
  GRAY_CMD_CLEAR_COUNTER = 0x01U,
  GRAY_CMD_UART_STREAM_ON = 0x02U,
  GRAY_CMD_UART_STREAM_OFF = 0x03U,
  GRAY_CMD_SOFT_RESET = 0x04U
} GrayCommandId;

typedef enum
{
  GRAY_SCENE_NONE = 0x00U,
  GRAY_SCENE_STRAIGHT = 0x01U,
  GRAY_SCENE_SOFT_LEFT = 0x02U,
  GRAY_SCENE_SOFT_RIGHT = 0x03U,
  GRAY_SCENE_HARD_LEFT = 0x04U,
  GRAY_SCENE_HARD_RIGHT = 0x05U,
  GRAY_SCENE_CROSS = 0x06U,
  GRAY_SCENE_LEFT_BRANCH = 0x07U,
  GRAY_SCENE_RIGHT_BRANCH = 0x08U,
  GRAY_SCENE_T_JUNCTION = 0x09U,
  GRAY_SCENE_STOP_BAR = 0x0AU,
  GRAY_SCENE_LINE_LOST = 0x0BU,
  GRAY_SCENE_AMBIGUOUS = 0x0CU
} GrayRuntimeScene;

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
  uint8_t cmd;
  uint8_t arg0;
  uint8_t arg1;
  uint8_t seq;
} GrayCommandFrame;

typedef struct
{
  int8_t turn;
  uint8_t scene;
  uint8_t flags;
} GrayRuntimeFrame;

void GrayProtocol_Init(void);
void GrayProtocol_ResetStatus(void);
void GrayProtocol_SetStatus(const GrayStatusFrame *status);
void GrayProtocol_SetStatusFlags(uint8_t flags);
void GrayProtocol_SetSensorBits(uint8_t sensor_bits);
void GrayProtocol_SetTrackType(uint8_t track_type);
void GrayProtocol_SetEdgeCount(uint8_t edge_count);
void GrayProtocol_SetCenterError(int16_t center_err);
void GrayProtocol_SetUartStreamEnabled(uint8_t enabled);
void GrayProtocol_CommitFrame(void);
const GrayStatusFrame *GrayProtocol_GetStatus(void);
void GrayProtocol_SetRuntimeTurn(int8_t turn);
void GrayProtocol_SetRuntimeScene(uint8_t scene);
void GrayProtocol_SetRuntimeFlags(uint8_t flags);
void GrayProtocol_SetRuntimeOutput(const GrayRuntimeFrame *runtime_frame);
const GrayRuntimeFrame *GrayProtocol_GetRuntimeOutput(void);

uint8_t GrayProtocol_ReadRegister(uint8_t reg_addr);
uint16_t GrayProtocol_FillRegisterWindow(uint8_t start_reg, uint8_t *buffer, uint16_t capacity);
void GrayProtocol_WriteRegister(uint8_t reg_addr, uint8_t value);
uint8_t GrayProtocol_FetchCommand(GrayCommandFrame *command);

uint16_t GrayProtocol_BuildStatusUartFrame(uint8_t *buffer, uint16_t capacity);
uint16_t GrayProtocol_BuildRuntimeShortFrame(uint8_t *buffer, uint16_t capacity);
uint8_t GrayProtocol_Crc8(const uint8_t *data, uint16_t length);
uint16_t GrayProtocol_Crc16CcittFalse(const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __GRAY_PROTOCOL_H__ */
