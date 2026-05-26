#include "openmv_vision.h"

#include "usart.h"

#include <string.h>

typedef enum
{
  OPENMV_RX_WAIT_SOF1 = 0,
  OPENMV_RX_WAIT_SOF2,
  OPENMV_RX_COLLECT_PAYLOAD,
  OPENMV_RX_COLLECT_CRC
} OpenMVRxState;

typedef struct
{
  OpenMVRxState     state;
  uint8_t           payload[OPENMV_VISION_PAYLOAD_LEN];
  uint8_t           payload_index;
  uint8_t           rx_byte;
  OpenMVVisionStats stats;
} OpenMVVisionContext;

static OpenMVVisionContext g_vision;

static uint8_t OpenMVVision_Crc8(const uint8_t *data, uint32_t length)
{
  uint8_t crc = 0U;

  while (length-- > 0U)
  {
    uint8_t bit;

    crc ^= *data++;
    for (bit = 0U; bit < 8U; ++bit)
    {
      if ((crc & 0x80U) != 0U)
      {
        crc = (uint8_t)((crc << 1) ^ 0x07U);
      }
      else
      {
        crc = (uint8_t)(crc << 1);
      }
    }
  }

  return crc;
}

static HAL_StatusTypeDef OpenMVVision_ArmReceive(void)
{
  return HAL_UART_Receive_IT(&huart4, &g_vision.rx_byte, 1U);
}

static void OpenMVVision_FinishFrame(uint8_t crc_received)
{
  OpenMVVisionFrame frame;
  uint8_t crc_calc;

  crc_calc = OpenMVVision_Crc8(g_vision.payload, OPENMV_VISION_PAYLOAD_LEN);
  if (crc_calc != crc_received)
  {
    g_vision.stats.rx_crc_errors++;
    g_vision.state = OPENMV_RX_WAIT_SOF1;
    g_vision.payload_index = 0U;
    return;
  }

  frame.x_offset = (int16_t)((uint16_t)g_vision.payload[0]
                             | ((uint16_t)g_vision.payload[1] << 8));
  frame.y_offset = (int16_t)((uint16_t)g_vision.payload[2]
                             | ((uint16_t)g_vision.payload[3] << 8));
  frame.width = (uint16_t)((uint16_t)g_vision.payload[4]
                           | ((uint16_t)g_vision.payload[5] << 8));
  frame.flags = g_vision.payload[6];
  frame.sequence = g_vision.payload[7];
  frame.timestamp_ms = HAL_GetTick();

  g_vision.stats.latest = frame;
  g_vision.stats.rx_frames++;
  g_vision.stats.online = 1U;

  g_vision.state = OPENMV_RX_WAIT_SOF1;
  g_vision.payload_index = 0U;
}

static void OpenMVVision_FeedByte(uint8_t byte)
{
  switch (g_vision.state)
  {
    case OPENMV_RX_WAIT_SOF1:
      if (byte == OPENMV_VISION_SOF1)
      {
        g_vision.state = OPENMV_RX_WAIT_SOF2;
      }
      else
      {
        g_vision.stats.rx_sync_drops++;
      }
      break;

    case OPENMV_RX_WAIT_SOF2:
      if (byte == OPENMV_VISION_SOF2)
      {
        g_vision.payload_index = 0U;
        g_vision.state = OPENMV_RX_COLLECT_PAYLOAD;
      }
      else if (byte == OPENMV_VISION_SOF1)
      {
        g_vision.stats.rx_sync_drops++;
      }
      else
      {
        g_vision.stats.rx_sync_drops++;
        g_vision.state = OPENMV_RX_WAIT_SOF1;
      }
      break;

    case OPENMV_RX_COLLECT_PAYLOAD:
      g_vision.payload[g_vision.payload_index++] = byte;
      if (g_vision.payload_index >= OPENMV_VISION_PAYLOAD_LEN)
      {
        g_vision.state = OPENMV_RX_COLLECT_CRC;
      }
      break;

    case OPENMV_RX_COLLECT_CRC:
    default:
      OpenMVVision_FinishFrame(byte);
      break;
  }
}

HAL_StatusTypeDef OpenMVVision_Init(void)
{
  (void)memset(&g_vision, 0, sizeof(g_vision));
  g_vision.state = OPENMV_RX_WAIT_SOF1;
  return OpenMVVision_ArmReceive();
}

void OpenMVVision_Poll(uint32_t now_ms)
{
  if (g_vision.stats.online == 0U)
  {
    return;
  }

  if ((now_ms - g_vision.stats.latest.timestamp_ms) > OPENMV_VISION_OFFLINE_TIMEOUT)
  {
    g_vision.stats.online = 0U;
  }
}

uint8_t OpenMVVision_HasFix(void)
{
  uint8_t flags;

  if (g_vision.stats.online == 0U)
  {
    return 0U;
  }

  flags = g_vision.stats.latest.flags;
  return (((flags & OPENMV_VISION_FLAG_VALID) != 0U)
          && ((flags & OPENMV_VISION_FLAG_RECT_FOUND) != 0U)) ? 1U : 0U;
}

uint8_t OpenMVVision_IsOnline(void)
{
  return g_vision.stats.online;
}

const OpenMVVisionFrame *OpenMVVision_GetLatest(void)
{
  return &g_vision.stats.latest;
}

const OpenMVVisionStats *OpenMVVision_GetStats(void)
{
  return &g_vision.stats;
}

void OpenMVVision_OnRxByte(void)
{
  OpenMVVision_FeedByte(g_vision.rx_byte);
  (void)OpenMVVision_ArmReceive();
}

void OpenMVVision_OnRxError(void)
{
  g_vision.state = OPENMV_RX_WAIT_SOF1;
  g_vision.payload_index = 0U;
  (void)OpenMVVision_ArmReceive();
}
