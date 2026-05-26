#include "gray_protocol.h"

#include <string.h>

#define GRAY_STATUS_PAYLOAD_LEN 8U
#define GRAY_FRAME_OVERHEAD     10U

static GrayStatusFrame g_gray_status;
static GrayCommandFrame g_pending_command;
static GrayRuntimeFrame g_runtime_frame;
static uint8_t g_command_pending = 0U;
static uint8_t g_uart_seq = 0U;

static void GrayProtocol_UpdateAutoFlags(void)
{
  g_gray_status.status_flags |= GRAY_STATUS_FLAG_READY;

  if ((g_gray_status.status_flags & GRAY_STATUS_FLAG_DATA_VALID) == 0U)
  {
    g_gray_status.status_flags &= (uint8_t)~GRAY_STATUS_FLAG_ALL_BLACK;
    g_gray_status.status_flags &= (uint8_t)~GRAY_STATUS_FLAG_ALL_WHITE;
    return;
  }

  if (g_gray_status.sensor_bits == 0xFFU)
  {
    g_gray_status.status_flags |= GRAY_STATUS_FLAG_ALL_BLACK;
  }
  else
  {
    g_gray_status.status_flags &= (uint8_t)~GRAY_STATUS_FLAG_ALL_BLACK;
  }

  if (g_gray_status.sensor_bits == 0x00U)
  {
    g_gray_status.status_flags |= GRAY_STATUS_FLAG_ALL_WHITE;
  }
  else
  {
    g_gray_status.status_flags &= (uint8_t)~GRAY_STATUS_FLAG_ALL_WHITE;
  }
}

static void GrayProtocol_ApplyImmediateCommand(void)
{
  switch (g_pending_command.cmd)
  {
    case GRAY_CMD_CLEAR_COUNTER:
      g_gray_status.frame_cnt = 0U;
      break;

    case GRAY_CMD_UART_STREAM_ON:
      GrayProtocol_SetUartStreamEnabled(1U);
      break;

    case GRAY_CMD_UART_STREAM_OFF:
      GrayProtocol_SetUartStreamEnabled(0U);
      break;

    case GRAY_CMD_NOP:
    case GRAY_CMD_SOFT_RESET:
    default:
      break;
  }
}

static uint16_t GrayProtocol_BuildFrame(uint8_t type,
                                        const uint8_t *payload,
                                        uint16_t payload_len,
                                        uint8_t *buffer,
                                        uint16_t capacity)
{
  uint16_t crc;
  uint16_t total_len;

  total_len = (uint16_t)(GRAY_FRAME_OVERHEAD + payload_len);
  if ((buffer == NULL) || (capacity < total_len) ||
      ((payload_len > 0U) && (payload == NULL)))
  {
    return 0U;
  }

  buffer[0] = GRAY_UART_SOF1;
  buffer[1] = GRAY_UART_SOF2;
  buffer[2] = GRAY_PROTOCOL_VERSION;
  buffer[3] = GRAY_UART_SRC_F103;
  buffer[4] = type;
  buffer[5] = g_uart_seq++;
  buffer[6] = (uint8_t)(payload_len & 0xFFU);
  buffer[7] = (uint8_t)((payload_len >> 8) & 0xFFU);

  if ((payload != NULL) && (payload_len > 0U))
  {
    memcpy(&buffer[8], payload, payload_len);
  }

  crc = GrayProtocol_Crc16CcittFalse(&buffer[2], (uint16_t)(6U + payload_len));
  buffer[8U + payload_len] = (uint8_t)(crc & 0xFFU);
  buffer[9U + payload_len] = (uint8_t)((crc >> 8) & 0xFFU);

  return total_len;
}

void GrayProtocol_Init(void)
{
  memset(&g_pending_command, 0, sizeof(g_pending_command));
  memset(&g_runtime_frame, 0, sizeof(g_runtime_frame));
  g_command_pending = 0U;
  g_uart_seq = 0U;
  GrayProtocol_ResetStatus();
}

void GrayProtocol_ResetStatus(void)
{
  memset(&g_gray_status, 0, sizeof(g_gray_status));
  g_gray_status.track_type = GRAY_TRACK_NONE;
  g_gray_status.status_flags = GRAY_STATUS_FLAG_READY;
  GrayProtocol_UpdateAutoFlags();
}

void GrayProtocol_SetStatus(const GrayStatusFrame *status)
{
  if (status == NULL)
  {
    return;
  }

  g_gray_status = *status;
  GrayProtocol_UpdateAutoFlags();
}

void GrayProtocol_SetStatusFlags(uint8_t flags)
{
  g_gray_status.status_flags = flags;
  GrayProtocol_UpdateAutoFlags();
}

void GrayProtocol_SetSensorBits(uint8_t sensor_bits)
{
  g_gray_status.sensor_bits = sensor_bits;
  g_gray_status.status_flags |= GRAY_STATUS_FLAG_DATA_VALID;
  GrayProtocol_UpdateAutoFlags();
}

void GrayProtocol_SetTrackType(uint8_t track_type)
{
  g_gray_status.track_type = track_type;
}

void GrayProtocol_SetEdgeCount(uint8_t edge_count)
{
  g_gray_status.edge_count = edge_count;
}

void GrayProtocol_SetCenterError(int16_t center_err)
{
  g_gray_status.center_err = center_err;
}

void GrayProtocol_SetUartStreamEnabled(uint8_t enabled)
{
  if (enabled != 0U)
  {
    g_gray_status.status_flags |= GRAY_STATUS_FLAG_UART_STREAM;
  }
  else
  {
    g_gray_status.status_flags &= (uint8_t)~GRAY_STATUS_FLAG_UART_STREAM;
  }
}

void GrayProtocol_CommitFrame(void)
{
  g_gray_status.frame_cnt++;
  GrayProtocol_UpdateAutoFlags();
}

const GrayStatusFrame *GrayProtocol_GetStatus(void)
{
  return &g_gray_status;
}

void GrayProtocol_SetRuntimeTurn(int8_t turn)
{
  g_runtime_frame.turn = turn;
}

void GrayProtocol_SetRuntimeScene(uint8_t scene)
{
  g_runtime_frame.scene = scene;
}

void GrayProtocol_SetRuntimeFlags(uint8_t flags)
{
  g_runtime_frame.flags = flags;
}

void GrayProtocol_SetRuntimeOutput(const GrayRuntimeFrame *runtime_frame)
{
  if (runtime_frame == NULL)
  {
    return;
  }

  g_runtime_frame = *runtime_frame;
}

const GrayRuntimeFrame *GrayProtocol_GetRuntimeOutput(void)
{
  return &g_runtime_frame;
}

uint8_t GrayProtocol_ReadRegister(uint8_t reg_addr)
{
  switch (reg_addr)
  {
    case GRAY_REG_DEV_ID:
      return GRAY_DEVICE_ID;

    case GRAY_REG_PROTO_VER:
      return GRAY_PROTOCOL_VERSION;

    case GRAY_REG_STATUS_FLAGS:
      return g_gray_status.status_flags;

    case GRAY_REG_SENSOR_BITS:
      return g_gray_status.sensor_bits;

    case GRAY_REG_TRACK_TYPE:
      return g_gray_status.track_type;

    case GRAY_REG_EDGE_COUNT:
      return g_gray_status.edge_count;

    case GRAY_REG_CENTER_ERR_L:
      return (uint8_t)((uint16_t)g_gray_status.center_err & 0xFFU);

    case GRAY_REG_CENTER_ERR_H:
      return (uint8_t)(((uint16_t)g_gray_status.center_err >> 8) & 0xFFU);

    case GRAY_REG_FRAME_CNT_L:
      return (uint8_t)(g_gray_status.frame_cnt & 0xFFU);

    case GRAY_REG_FRAME_CNT_H:
      return (uint8_t)((g_gray_status.frame_cnt >> 8) & 0xFFU);

    case GRAY_REG_CMD:
      return g_pending_command.cmd;

    case GRAY_REG_CMD_ARG0:
      return g_pending_command.arg0;

    case GRAY_REG_CMD_ARG1:
      return g_pending_command.arg1;

    case GRAY_REG_CMD_SEQ:
      return g_pending_command.seq;

    default:
      return 0U;
  }
}

uint16_t GrayProtocol_FillRegisterWindow(uint8_t start_reg, uint8_t *buffer, uint16_t capacity)
{
  uint16_t index;

  if (buffer == NULL)
  {
    return 0U;
  }

  for (index = 0U; index < capacity; ++index)
  {
    buffer[index] = GrayProtocol_ReadRegister((uint8_t)(start_reg + index));
  }

  return capacity;
}

void GrayProtocol_WriteRegister(uint8_t reg_addr, uint8_t value)
{
  switch (reg_addr)
  {
    case GRAY_REG_CMD:
      g_pending_command.cmd = value;
      break;

    case GRAY_REG_CMD_ARG0:
      g_pending_command.arg0 = value;
      break;

    case GRAY_REG_CMD_ARG1:
      g_pending_command.arg1 = value;
      break;

    case GRAY_REG_CMD_SEQ:
      g_pending_command.seq = value;
      g_command_pending = 1U;
      GrayProtocol_ApplyImmediateCommand();
      break;

    default:
      break;
  }
}

uint8_t GrayProtocol_FetchCommand(GrayCommandFrame *command)
{
  if ((g_command_pending == 0U) || (command == NULL))
  {
    return 0U;
  }

  *command = g_pending_command;
  g_command_pending = 0U;
  return 1U;
}

uint16_t GrayProtocol_BuildStatusUartFrame(uint8_t *buffer, uint16_t capacity)
{
  uint8_t payload[GRAY_STATUS_PAYLOAD_LEN];

  payload[0] = g_gray_status.status_flags;
  payload[1] = g_gray_status.sensor_bits;
  payload[2] = g_gray_status.track_type;
  payload[3] = g_gray_status.edge_count;
  payload[4] = (uint8_t)((uint16_t)g_gray_status.center_err & 0xFFU);
  payload[5] = (uint8_t)(((uint16_t)g_gray_status.center_err >> 8) & 0xFFU);
  payload[6] = (uint8_t)(g_gray_status.frame_cnt & 0xFFU);
  payload[7] = (uint8_t)((g_gray_status.frame_cnt >> 8) & 0xFFU);

  return GrayProtocol_BuildFrame(GRAY_UART_TYPE_STATUS, payload, GRAY_STATUS_PAYLOAD_LEN, buffer, capacity);
}

uint16_t GrayProtocol_BuildRuntimeShortFrame(uint8_t *buffer, uint16_t capacity)
{
  if ((buffer == NULL) || (capacity < GRAY_RUNTIME_FRAME_LEN))
  {
    return 0U;
  }

  buffer[0] = GRAY_UART_SOF1;
  buffer[1] = GRAY_UART_SOF2;
  buffer[2] = (uint8_t)g_runtime_frame.turn;
  buffer[3] = g_runtime_frame.scene;
  buffer[4] = g_runtime_frame.flags;
  buffer[5] = GrayProtocol_Crc8(&buffer[2], 3U);

  return GRAY_RUNTIME_FRAME_LEN;
}

uint8_t GrayProtocol_Crc8(const uint8_t *data, uint16_t length)
{
  uint8_t crc = 0x00U;
  uint16_t index;
  uint8_t bit;

  if (data == NULL)
  {
    return 0U;
  }

  for (index = 0U; index < length; ++index)
  {
    crc ^= data[index];

    for (bit = 0U; bit < 8U; ++bit)
    {
      if ((crc & 0x80U) != 0U)
      {
        crc = (uint8_t)((crc << 1) ^ 0x07U);
      }
      else
      {
        crc <<= 1;
      }
    }
  }

  return crc;
}

uint16_t GrayProtocol_Crc16CcittFalse(const uint8_t *data, uint16_t length)
{
  uint16_t crc = 0xFFFFU;
  uint16_t i;
  uint8_t bit;

  if (data == NULL)
  {
    return 0U;
  }

  for (i = 0U; i < length; ++i)
  {
    crc ^= (uint16_t)((uint16_t)data[i] << 8);

    for (bit = 0U; bit < 8U; ++bit)
    {
      if ((crc & 0x8000U) != 0U)
      {
        crc = (uint16_t)((crc << 1) ^ 0x1021U);
      }
      else
      {
        crc <<= 1;
      }
    }
  }

  return crc;
}
