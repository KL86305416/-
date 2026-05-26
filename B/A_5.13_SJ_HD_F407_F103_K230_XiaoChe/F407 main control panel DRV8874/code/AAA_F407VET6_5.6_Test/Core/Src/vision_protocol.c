#include "vision_protocol.h"

#include <string.h>

#define UART_PROTOCOL_FRAME_OVERHEAD 10U

static uint16_t VisionProtocol_ReadU16Le(const uint8_t *data)
{
  return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

static int16_t VisionProtocol_ReadS16Le(const uint8_t *data)
{
  return (int16_t)VisionProtocol_ReadU16Le(data);
}

void VisionModule_Init(VisionModuleState *state)
{
  if (state == NULL)
  {
    return;
  }

  memset(state, 0, sizeof(*state));
}

uint8_t VisionModule_HandleFrame(VisionModuleState *state,
                                 const UartProtocolFrame *frame,
                                 uint32_t tick_ms)
{
  if ((state == NULL) || (frame == NULL) || (frame->payload == NULL))
  {
    return 0U;
  }

  if (frame->src != UART_PROTOCOL_SRC_OPENMV)
  {
    return 0U;
  }

  state->online = 1U;
  state->ready = 1U;
  state->last_type = frame->type;
  state->last_seq = frame->seq;
  state->last_rx_ms = tick_ms;

  switch (frame->type)
  {
    case VISION_MSG_HELLO:
      if (frame->payload_len != 5U)
      {
        return 0U;
      }

      state->hello.dev_id = frame->payload[0];
      state->hello.fw_major = frame->payload[1];
      state->hello.fw_minor = frame->payload[2];
      state->hello.capability_mask = VisionProtocol_ReadU16Le(&frame->payload[3]);
      return 1U;

    case VISION_MSG_STATUS:
      if (frame->payload_len != 10U)
      {
        return 0U;
      }

      state->status.state = frame->payload[0];
      state->status.mode = frame->payload[1];
      state->status.fps_x10 = VisionProtocol_ReadU16Le(&frame->payload[2]);
      state->status.latency_ms = VisionProtocol_ReadU16Le(&frame->payload[4]);
      state->status.err_flags = VisionProtocol_ReadU16Le(&frame->payload[6]);
      state->status.uptime_s = VisionProtocol_ReadU16Le(&frame->payload[8]);
      return 1U;

    case VISION_MSG_RESULT:
      if (frame->payload_len != 20U)
      {
        return 0U;
      }

      state->result.mode = frame->payload[0];
      state->result.target_count = frame->payload[1];
      state->result.label = frame->payload[2];
      state->result.confidence = frame->payload[3];
      state->result.cx = VisionProtocol_ReadS16Le(&frame->payload[4]);
      state->result.cy = VisionProtocol_ReadS16Le(&frame->payload[6]);
      state->result.w = VisionProtocol_ReadU16Le(&frame->payload[8]);
      state->result.h = VisionProtocol_ReadU16Le(&frame->payload[10]);
      state->result.angle_x10 = VisionProtocol_ReadS16Le(&frame->payload[12]);
      state->result.distance_x10 = VisionProtocol_ReadU16Le(&frame->payload[14]);
      state->result.flags = VisionProtocol_ReadU16Le(&frame->payload[16]);
      state->result.frame_id = VisionProtocol_ReadU16Le(&frame->payload[18]);
      return 1U;

    case VISION_MSG_DEBUG_TEXT:
    case VISION_MSG_ERROR_REPORT:
    case VISION_MSG_ACK:
      return 1U;

    default:
      return 0U;
  }
}

uint8_t VisionModule_IsOnline(const VisionModuleState *state, uint32_t now_ms, uint32_t timeout_ms)
{
  if (state == NULL)
  {
    return 0U;
  }

  if (state->online == 0U)
  {
    return 0U;
  }

  if ((now_ms - state->last_rx_ms) > timeout_ms)
  {
    return 0U;
  }

  return 1U;
}

uint16_t VisionProtocol_BuildFrame(uint8_t src,
                                   uint8_t type,
                                   uint8_t seq,
                                   const uint8_t *payload,
                                   uint16_t payload_len,
                                   uint8_t *buffer,
                                   uint16_t capacity)
{
  uint16_t crc;
  uint16_t total_len;

  total_len = (uint16_t)(UART_PROTOCOL_FRAME_OVERHEAD + payload_len);
  if ((buffer == NULL) || (capacity < total_len) ||
      ((payload_len > 0U) && (payload == NULL)))
  {
    return 0U;
  }

  buffer[0] = UART_PROTOCOL_SOF1;
  buffer[1] = UART_PROTOCOL_SOF2;
  buffer[2] = UART_PROTOCOL_VERSION;
  buffer[3] = src;
  buffer[4] = type;
  buffer[5] = seq;
  buffer[6] = (uint8_t)(payload_len & 0xFFU);
  buffer[7] = (uint8_t)((payload_len >> 8) & 0xFFU);

  if ((payload_len > 0U) && (payload != NULL))
  {
    memcpy(&buffer[8], payload, payload_len);
  }

  crc = VisionProtocol_Crc16CcittFalse(&buffer[2], (uint16_t)(6U + payload_len));
  buffer[8U + payload_len] = (uint8_t)(crc & 0xFFU);
  buffer[9U + payload_len] = (uint8_t)((crc >> 8) & 0xFFU);

  return total_len;
}

uint16_t VisionProtocol_BuildAckFrame(uint8_t seq,
                                      uint8_t acked_type,
                                      uint8_t result,
                                      uint8_t *buffer,
                                      uint16_t capacity)
{
  uint8_t payload[2];

  payload[0] = acked_type;
  payload[1] = result;

  return VisionProtocol_BuildFrame(UART_PROTOCOL_SRC_F407,
                                   VISION_MSG_ACK,
                                   seq,
                                   payload,
                                   2U,
                                   buffer,
                                   capacity);
}

uint16_t VisionProtocol_BuildModeSetFrame(uint8_t seq,
                                          uint8_t mode,
                                          uint8_t *buffer,
                                          uint16_t capacity)
{
  return VisionProtocol_BuildFrame(UART_PROTOCOL_SRC_F407,
                                   VISION_MSG_MODE_SET,
                                   seq,
                                   &mode,
                                   1U,
                                   buffer,
                                   capacity);
}

uint16_t VisionProtocol_BuildRunCtrlFrame(uint8_t seq,
                                          uint8_t run,
                                          uint8_t *buffer,
                                          uint16_t capacity)
{
  return VisionProtocol_BuildFrame(UART_PROTOCOL_SRC_F407,
                                   VISION_MSG_RUN_CTRL,
                                   seq,
                                   &run,
                                   1U,
                                   buffer,
                                   capacity);
}

uint8_t VisionProtocol_ParseFrame(const uint8_t *data, uint16_t length, UartProtocolFrame *frame)
{
  uint16_t payload_len;
  uint16_t expected_len;
  uint16_t crc_expected;
  uint16_t crc_actual;

  if ((data == NULL) || (frame == NULL))
  {
    return 0U;
  }

  if (length < UART_PROTOCOL_FRAME_OVERHEAD)
  {
    return 0U;
  }

  if ((data[0] != UART_PROTOCOL_SOF1) || (data[1] != UART_PROTOCOL_SOF2))
  {
    return 0U;
  }

  if (data[2] != UART_PROTOCOL_VERSION)
  {
    return 0U;
  }

  payload_len = VisionProtocol_ReadU16Le(&data[6]);
  expected_len = (uint16_t)(UART_PROTOCOL_FRAME_OVERHEAD + payload_len);
  if (length != expected_len)
  {
    return 0U;
  }

  crc_expected = VisionProtocol_ReadU16Le(&data[8U + payload_len]);
  crc_actual = VisionProtocol_Crc16CcittFalse(&data[2], (uint16_t)(6U + payload_len));
  if (crc_actual != crc_expected)
  {
    return 0U;
  }

  frame->src = data[3];
  frame->type = data[4];
  frame->seq = data[5];
  frame->payload_len = payload_len;
  frame->payload = &data[8];

  return 1U;
}

uint16_t VisionProtocol_Crc16CcittFalse(const uint8_t *data, uint16_t length)
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
