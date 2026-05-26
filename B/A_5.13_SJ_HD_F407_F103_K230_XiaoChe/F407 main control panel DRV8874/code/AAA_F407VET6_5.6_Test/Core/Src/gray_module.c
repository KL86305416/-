#include "gray_module.h"

#include <string.h>

static uint16_t GrayModule_ReadU16Le(const uint8_t *data)
{
  return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

static int16_t GrayModule_ReadS16Le(const uint8_t *data)
{
  return (int16_t)GrayModule_ReadU16Le(data);
}

static void GrayModule_UpdateRuntimeOutput(GrayModuleState *state,
                                           int8_t turn,
                                           uint8_t scene,
                                           uint8_t flags,
                                           uint32_t tick_ms)
{
  state->online = 1U;
  state->runtime.turn = turn;
  state->runtime.scene = scene;
  state->runtime.flags = flags;
  state->last_update_ms = tick_ms;
}

void GrayModule_Init(GrayModuleState *state)
{
  if (state == NULL)
  {
    return;
  }

  memset(state, 0, sizeof(*state));
}

uint8_t GrayModule_ParseI2cBlock(const uint8_t *data,
                                 uint16_t length,
                                 GrayModuleState *state,
                                 uint32_t tick_ms)
{
  if ((data == NULL) || (state == NULL))
  {
    return 0U;
  }

  if (length < GRAY_MODULE_I2C_BLOCK_LEN)
  {
    return 0U;
  }

  if ((data[GRAY_MODULE_REG_DEV_ID] != GRAY_MODULE_DEVICE_ID) ||
      (data[GRAY_MODULE_REG_PROTO_VER] != GRAY_MODULE_PROTOCOL_VERSION))
  {
    return 0U;
  }

  state->online = 1U;
  state->status.status_flags = data[GRAY_MODULE_REG_STATUS_FLAGS];
  state->status.sensor_bits = data[GRAY_MODULE_REG_SENSOR_BITS];
  state->status.track_type = data[GRAY_MODULE_REG_TRACK_TYPE];
  state->status.edge_count = data[GRAY_MODULE_REG_EDGE_COUNT];
  state->status.center_err = GrayModule_ReadS16Le(&data[GRAY_MODULE_REG_CENTER_ERR_L]);
  state->status.frame_cnt = GrayModule_ReadU16Le(&data[GRAY_MODULE_REG_FRAME_CNT_L]);
  state->last_update_ms = tick_ms;

  return 1U;
}

uint16_t GrayModule_BuildI2cCommand(uint8_t cmd,
                                    uint8_t arg0,
                                    uint8_t arg1,
                                    uint8_t seq,
                                    uint8_t *buffer,
                                    uint16_t capacity)
{
  if ((buffer == NULL) || (capacity < 4U))
  {
    return 0U;
  }

  buffer[0] = cmd;
  buffer[1] = arg0;
  buffer[2] = arg1;
  buffer[3] = seq;

  return 4U;
}

uint8_t GrayModule_ParseUartStatusPayload(const uint8_t *payload,
                                          uint16_t length,
                                          GrayModuleState *state,
                                          uint32_t tick_ms)
{
  if ((payload == NULL) || (state == NULL))
  {
    return 0U;
  }

  if (length < GRAY_MODULE_UART_STATUS_LEN)
  {
    return 0U;
  }

  state->online = 1U;
  state->status.status_flags = payload[0];
  state->status.sensor_bits = payload[1];
  state->status.track_type = payload[2];
  state->status.edge_count = payload[3];
  state->status.center_err = GrayModule_ReadS16Le(&payload[4]);
  state->status.frame_cnt = GrayModule_ReadU16Le(&payload[6]);
  state->last_update_ms = tick_ms;

  return 1U;
}

uint8_t GrayModule_ParseRuntimeFrame(const uint8_t *frame,
                                     uint16_t length,
                                     GrayModuleState *state,
                                     uint32_t tick_ms)
{
  uint8_t crc;

  if ((frame == NULL) || (state == NULL))
  {
    return 0U;
  }

  if (length < GRAY_MODULE_RUNTIME_FRAME_LEN)
  {
    return 0U;
  }

  if ((frame[0] != GRAY_MODULE_UART_SOF1) || (frame[1] != GRAY_MODULE_UART_SOF2))
  {
    return 0U;
  }

  crc = GrayModule_Crc8(&frame[2], 3U);
  if (crc != frame[5])
  {
    return 0U;
  }

  GrayModule_UpdateRuntimeOutput(state,
                                 (int8_t)frame[2],
                                 frame[3],
                                 frame[4],
                                 tick_ms);

  return 1U;
}

uint8_t GrayModule_ConsumeRuntimeUartByte(GrayModuleState *state,
                                          uint8_t byte,
                                          uint32_t tick_ms)
{
  if (state == NULL)
  {
    return 0U;
  }

  if (state->rx_index == 0U)
  {
    if (byte == GRAY_MODULE_UART_SOF1)
    {
      state->rx_buffer[0] = byte;
      state->rx_index = 1U;
    }

    return 0U;
  }

  if (state->rx_index == 1U)
  {
    if (byte == GRAY_MODULE_UART_SOF2)
    {
      state->rx_buffer[1] = byte;
      state->rx_index = 2U;
    }
    else if (byte == GRAY_MODULE_UART_SOF1)
    {
      state->rx_buffer[0] = byte;
      state->rx_index = 1U;
    }
    else
    {
      state->rx_index = 0U;
    }

    return 0U;
  }

  state->rx_buffer[state->rx_index] = byte;
  state->rx_index++;

  if (state->rx_index >= GRAY_MODULE_RUNTIME_FRAME_LEN)
  {
    state->rx_index = 0U;
    return GrayModule_ParseRuntimeFrame(state->rx_buffer,
                                        GRAY_MODULE_RUNTIME_FRAME_LEN,
                                        state,
                                        tick_ms);
  }

  return 0U;
}

void GrayModule_Service(GrayModuleState *state, uint32_t now_ms, uint32_t timeout_ms)
{
  if (state == NULL)
  {
    return;
  }

  if ((state->online != 0U) && ((now_ms - state->last_update_ms) > timeout_ms))
  {
    state->online = 0U;
  }
}

uint8_t GrayModule_IsOnline(const GrayModuleState *state, uint32_t now_ms, uint32_t timeout_ms)
{
  if (state == NULL)
  {
    return 0U;
  }

  if (state->online == 0U)
  {
    return 0U;
  }

  if ((now_ms - state->last_update_ms) > timeout_ms)
  {
    return 0U;
  }

  return 1U;
}

uint8_t GrayModule_Crc8(const uint8_t *data, uint16_t length)
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
        crc = (uint8_t)((crc << 1U) ^ 0x07U);
      }
      else
      {
        crc <<= 1U;
      }
    }
  }

  return crc;
}
