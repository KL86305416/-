#include "x42s_can_bus.h"

#include "can.h"

#include <string.h>

#define X42S_CAN_FILTER_SLAVE_START_BANK 14U
#define X42S_CAN1_FILTER_BANK            0U
#define X42S_CAN2_FILTER_BANK            14U
#define X42S_CAN_BROADCAST_ADDR          0U
#define X42S_CAN_MAX_PACKET_DATA         8U

#define X42S_CMD_READ_VERSION            0x1FU
#define X42S_CMD_READ_SPEED              0x35U
#define X42S_CMD_READ_POSITION           0x36U
#define X42S_CMD_READ_STATUS             0x3AU
#define X42S_CMD_READ_HOMING_STATUS      0x3BU
#define X42S_CMD_ENABLE                  0xF3U
#define X42S_CMD_SPEED_MODE              0xF6U
#define X42S_CMD_POSITION_MODE           0xFDU
#define X42S_CMD_STOP_NOW                0xFEU
#define X42S_CMD_TRIGGER_SYNC            0xFFU

static X42sMotorNode g_nodes[X42S_MOTOR_COUNT];
static X42sProtocolTypeDef g_protocol = X42S_DEFAULT_PROTOCOL;

static uint32_t X42sCanBus_MakeExtId(uint8_t addr, uint8_t packet)
{
  return (((uint32_t)addr << 8) | packet);
}

static void X42sCanBus_InitNodes(void)
{
  (void)memset(g_nodes, 0, sizeof(g_nodes));

  /* PCB netlist mapping:
     - 步进电机1 connector routes to CAN2
     - 步进电机2 connector routes to CAN1 */
  g_nodes[X42S_MOTOR_1].hcan = &hcan2;
  g_nodes[X42S_MOTOR_1].bus_id = 2U;
  g_nodes[X42S_MOTOR_1].motor_addr = X42S_MOTOR1_ADDR;

  g_nodes[X42S_MOTOR_2].hcan = &hcan1;
  g_nodes[X42S_MOTOR_2].bus_id = 1U;
  g_nodes[X42S_MOTOR_2].motor_addr = X42S_MOTOR2_ADDR;
}

static X42sMotorNode *X42sCanBus_GetMotorNodeMutable(X42sMotorId motor_id)
{
  if ((uint32_t)motor_id >= (uint32_t)X42S_MOTOR_COUNT)
  {
    return NULL;
  }

  return &g_nodes[(uint32_t)motor_id];
}

static X42sMotorNode *X42sCanBus_FindNode(CAN_HandleTypeDef *hcan, uint8_t addr)
{
  uint32_t index;

  for (index = 0U; index < (uint32_t)X42S_MOTOR_COUNT; ++index)
  {
    if ((g_nodes[index].hcan == hcan) && (g_nodes[index].motor_addr == addr))
    {
      return &g_nodes[index];
    }
  }

  return NULL;
}

static HAL_StatusTypeDef X42sCanBus_ConfigFilter(CAN_HandleTypeDef *hcan, uint32_t filter_bank)
{
  CAN_FilterTypeDef filter = {0};

  filter.FilterBank = filter_bank;
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_32BIT;
  filter.FilterIdHigh = 0U;
  filter.FilterIdLow = 0U;
  filter.FilterMaskIdHigh = 0U;
  filter.FilterMaskIdLow = 0U;
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  filter.FilterActivation = ENABLE;
  filter.SlaveStartFilterBank = X42S_CAN_FILTER_SLAVE_START_BANK;

  return HAL_CAN_ConfigFilter(hcan, &filter);
}

static HAL_StatusTypeDef X42sCanBus_StartOne(CAN_HandleTypeDef *hcan, uint32_t filter_bank)
{
  HAL_StatusTypeDef status;

  status = X42sCanBus_ConfigFilter(hcan, filter_bank);
  if (status != HAL_OK)
  {
    return status;
  }

  status = HAL_CAN_Start(hcan);
  if (status != HAL_OK)
  {
    return status;
  }

  status = HAL_CAN_ActivateNotification(hcan,
                                        CAN_IT_RX_FIFO0_MSG_PENDING |
                                        CAN_IT_ERROR_WARNING |
                                        CAN_IT_ERROR_PASSIVE |
                                        CAN_IT_BUSOFF |
                                        CAN_IT_LAST_ERROR_CODE |
                                        CAN_IT_ERROR);
  return status;
}

static HAL_StatusTypeDef X42sCanBus_SendStream(CAN_HandleTypeDef *hcan,
                                               uint8_t addr,
                                               const uint8_t *stream,
                                               uint8_t stream_len,
                                               X42sMotorNode *owner)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  CAN_TxHeaderTypeDef header = {0};
  uint8_t packet = 0U;
  uint16_t offset = 0U;

  if ((hcan == NULL) || (stream == NULL) || (stream_len == 0U))
  {
    return HAL_ERROR;
  }

  header.IDE = CAN_ID_EXT;
  header.RTR = CAN_RTR_DATA;
  header.TransmitGlobalTime = DISABLE;

  while (offset < stream_len)
  {
    uint8_t chunk_len;
    uint32_t mailbox = 0U;

    if (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0U)
    {
      status = HAL_BUSY;
      break;
    }

    chunk_len = (uint8_t)(stream_len - offset);
    if (chunk_len > X42S_CAN_MAX_PACKET_DATA)
    {
      chunk_len = X42S_CAN_MAX_PACKET_DATA;
    }

    header.ExtId = X42sCanBus_MakeExtId(addr, packet);
    header.DLC = chunk_len;

    status = HAL_CAN_AddTxMessage(hcan, &header, (uint8_t *)&stream[offset], &mailbox);
    if (status != HAL_OK)
    {
      break;
    }

    offset = (uint16_t)(offset + chunk_len);
    packet++;
  }

  if (owner != NULL)
  {
    owner->last_error = HAL_CAN_GetError(hcan);

    if (status == HAL_OK)
    {
      owner->last_tx_cmd = stream[0];
      owner->last_tx_stream_len = stream_len;
      owner->last_tx_packet_count = packet;
      owner->last_tx_ext_id = X42sCanBus_MakeExtId(addr, 0U);
      owner->last_tx_tick = HAL_GetTick();

      if (stream_len > (uint8_t)sizeof(owner->last_tx_stream))
      {
        stream_len = (uint8_t)sizeof(owner->last_tx_stream);
      }

      (void)memcpy(owner->last_tx_stream, stream, stream_len);
    }
  }

  return status;
}

static HAL_StatusTypeDef X42sCanBus_SendToMotor(X42sMotorNode *node,
                                                const uint8_t *stream,
                                                uint8_t stream_len)
{
  if (node == NULL)
  {
    return HAL_ERROR;
  }

  return X42sCanBus_SendStream(node->hcan, node->motor_addr, stream, stream_len, node);
}

static uint32_t X42sCanBus_Abs32(int32_t value)
{
  if (value == ((int32_t)0x80000000UL))
  {
    return 0x80000000UL;
  }

  if (value < 0)
  {
    return (uint32_t)(-value);
  }

  return (uint32_t)value;
}

HAL_StatusTypeDef X42sCanBus_Init(void)
{
  X42sCanBus_InitNodes();

  if (X42sCanBus_StartOne(&hcan1, X42S_CAN1_FILTER_BANK) != HAL_OK)
  {
    return HAL_ERROR;
  }

  if (X42sCanBus_StartOne(&hcan2, X42S_CAN2_FILTER_BANK) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

void X42sCanBus_Poll(uint32_t now_ms)
{
  uint32_t index;

  for (index = 0U; index < (uint32_t)X42S_MOTOR_COUNT; ++index)
  {
    if ((g_nodes[index].online != 0U) &&
        ((now_ms - g_nodes[index].last_rx_tick) > X42S_MOTOR_OFFLINE_TIMEOUT_MS))
    {
      g_nodes[index].online = 0U;
    }

    if (g_nodes[index].hcan != NULL)
    {
      uint32_t error_code = HAL_CAN_GetError(g_nodes[index].hcan);
      if (error_code != HAL_CAN_ERROR_NONE)
      {
        g_nodes[index].last_error = error_code;
      }
    }
  }
}

const X42sMotorNode *X42sCanBus_GetMotorNode(X42sMotorId motor_id)
{
  return X42sCanBus_GetMotorNodeMutable(motor_id);
}

X42sProtocolTypeDef X42sCanBus_GetProtocol(void)
{
  return g_protocol;
}

const char *X42sCanBus_GetProtocolName(void)
{
  return (g_protocol == X42S_PROTOCOL_X) ? "X" : "EMM";
}

HAL_StatusTypeDef X42sCanBus_ReadVersion(X42sMotorId motor_id)
{
  static const uint8_t payload[] = {X42S_CMD_READ_VERSION};

  return X42sCanBus_SendToMotor(X42sCanBus_GetMotorNodeMutable(motor_id),
                                payload,
                                (uint8_t)sizeof(payload));
}

HAL_StatusTypeDef X42sCanBus_ReadSpeed(X42sMotorId motor_id)
{
  static const uint8_t payload[] = {X42S_CMD_READ_SPEED};

  return X42sCanBus_SendToMotor(X42sCanBus_GetMotorNodeMutable(motor_id),
                                payload,
                                (uint8_t)sizeof(payload));
}

HAL_StatusTypeDef X42sCanBus_ReadPosition(X42sMotorId motor_id)
{
  static const uint8_t payload[] = {X42S_CMD_READ_POSITION};

  return X42sCanBus_SendToMotor(X42sCanBus_GetMotorNodeMutable(motor_id),
                                payload,
                                (uint8_t)sizeof(payload));
}

HAL_StatusTypeDef X42sCanBus_ReadStatus(X42sMotorId motor_id)
{
  static const uint8_t payload[] = {X42S_CMD_READ_STATUS};

  return X42sCanBus_SendToMotor(X42sCanBus_GetMotorNodeMutable(motor_id),
                                payload,
                                (uint8_t)sizeof(payload));
}

HAL_StatusTypeDef X42sCanBus_ReadHomingStatus(X42sMotorId motor_id)
{
  static const uint8_t payload[] = {X42S_CMD_READ_HOMING_STATUS};

  return X42sCanBus_SendToMotor(X42sCanBus_GetMotorNodeMutable(motor_id),
                                payload,
                                (uint8_t)sizeof(payload));
}

HAL_StatusTypeDef X42sCanBus_Enable(X42sMotorId motor_id, uint8_t enable, uint8_t sync)
{
  uint8_t payload[4];

  payload[0] = X42S_CMD_ENABLE;
  payload[1] = 0xABU;
  payload[2] = (enable != 0U) ? 0x01U : 0x00U;
  payload[3] = (sync != 0U) ? 0x01U : 0x00U;

  return X42sCanBus_SendToMotor(X42sCanBus_GetMotorNodeMutable(motor_id),
                                payload,
                                (uint8_t)sizeof(payload));
}

HAL_StatusTypeDef X42sCanBus_Stop(X42sMotorId motor_id, uint8_t sync)
{
  uint8_t payload[3];

  payload[0] = X42S_CMD_STOP_NOW;
  payload[1] = 0x98U;
  payload[2] = (sync != 0U) ? 0x01U : 0x00U;

  return X42sCanBus_SendToMotor(X42sCanBus_GetMotorNodeMutable(motor_id),
                                payload,
                                (uint8_t)sizeof(payload));
}

HAL_StatusTypeDef X42sCanBus_TriggerSync(void)
{
  static const uint8_t payload[] = {X42S_CMD_TRIGGER_SYNC, 0x66U};
  HAL_StatusTypeDef status_1;
  HAL_StatusTypeDef status_2;

  status_1 = X42sCanBus_SendStream(&hcan1,
                                   X42S_CAN_BROADCAST_ADDR,
                                   payload,
                                   (uint8_t)sizeof(payload),
                                   NULL);
  status_2 = X42sCanBus_SendStream(&hcan2,
                                   X42S_CAN_BROADCAST_ADDR,
                                   payload,
                                   (uint8_t)sizeof(payload),
                                   NULL);

  if ((status_1 == HAL_OK) && (status_2 == HAL_OK))
  {
    return HAL_OK;
  }

  return HAL_ERROR;
}

HAL_StatusTypeDef X42sCanBus_SetVelocity(X42sMotorId motor_id,
                                         uint8_t direction,
                                         uint16_t velocity,
                                         uint16_t ramp,
                                         uint8_t sync)
{
  uint8_t payload[7];
  uint8_t length = 0U;

  if (g_protocol == X42S_PROTOCOL_EMM)
  {
    payload[0] = X42S_CMD_SPEED_MODE;
    payload[1] = direction;
    payload[2] = (uint8_t)((velocity >> 8) & 0xFFU);
    payload[3] = (uint8_t)(velocity & 0xFFU);
    payload[4] = (uint8_t)(ramp & 0xFFU);
    payload[5] = (sync != 0U) ? 0x01U : 0x00U;
    length = 6U;
  }
  else
  {
    payload[0] = X42S_CMD_SPEED_MODE;
    payload[1] = direction;
    payload[2] = (uint8_t)((ramp >> 8) & 0xFFU);
    payload[3] = (uint8_t)(ramp & 0xFFU);
    payload[4] = (uint8_t)((velocity >> 8) & 0xFFU);
    payload[5] = (uint8_t)(velocity & 0xFFU);
    payload[6] = (sync != 0U) ? 0x01U : 0x00U;
    length = 7U;
  }

  return X42sCanBus_SendToMotor(X42sCanBus_GetMotorNodeMutable(motor_id), payload, length);
}

HAL_StatusTypeDef X42sCanBus_SetPosition(X42sMotorId motor_id,
                                         uint8_t direction,
                                         uint16_t velocity,
                                         uint16_t accel,
                                         uint16_t decel,
                                         int32_t target,
                                         uint8_t mode,
                                         uint8_t sync)
{
  uint8_t payload[14];
  uint8_t length = 0U;
  uint32_t target_u32 = X42sCanBus_Abs32(target);

  if (g_protocol == X42S_PROTOCOL_EMM)
  {
    payload[0] = X42S_CMD_POSITION_MODE;
    payload[1] = direction;
    payload[2] = (uint8_t)((velocity >> 8) & 0xFFU);
    payload[3] = (uint8_t)(velocity & 0xFFU);
    payload[4] = (uint8_t)(accel & 0xFFU);
    payload[5] = (uint8_t)((target_u32 >> 24) & 0xFFU);
    payload[6] = (uint8_t)((target_u32 >> 16) & 0xFFU);
    payload[7] = (uint8_t)((target_u32 >> 8) & 0xFFU);
    payload[8] = (uint8_t)(target_u32 & 0xFFU);
    payload[9] = mode;
    payload[10] = (sync != 0U) ? 0x01U : 0x00U;
    length = 11U;
  }
  else
  {
    payload[0] = X42S_CMD_POSITION_MODE;
    payload[1] = direction;
    payload[2] = (uint8_t)((accel >> 8) & 0xFFU);
    payload[3] = (uint8_t)(accel & 0xFFU);
    payload[4] = (uint8_t)((decel >> 8) & 0xFFU);
    payload[5] = (uint8_t)(decel & 0xFFU);
    payload[6] = (uint8_t)((velocity >> 8) & 0xFFU);
    payload[7] = (uint8_t)(velocity & 0xFFU);
    payload[8] = (uint8_t)((target_u32 >> 24) & 0xFFU);
    payload[9] = (uint8_t)((target_u32 >> 16) & 0xFFU);
    payload[10] = (uint8_t)((target_u32 >> 8) & 0xFFU);
    payload[11] = (uint8_t)(target_u32 & 0xFFU);
    payload[12] = mode;
    payload[13] = (sync != 0U) ? 0x01U : 0x00U;
    length = 14U;
  }

  return X42sCanBus_SendToMotor(X42sCanBus_GetMotorNodeMutable(motor_id), payload, length);
}

void X42sCanBus_OnRxPending(CAN_HandleTypeDef *hcan)
{
  while ((hcan != NULL) && (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0U))
  {
    CAN_RxHeaderTypeDef header = {0};
    uint8_t data[8];
    uint8_t addr;
    uint8_t packet;
    uint16_t offset;
    X42sMotorNode *node;

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &header, data) != HAL_OK)
    {
      return;
    }

    if (header.IDE != CAN_ID_EXT)
    {
      continue;
    }

    addr = (uint8_t)((header.ExtId >> 8) & 0xFFU);
    packet = (uint8_t)(header.ExtId & 0xFFU);
    node = X42sCanBus_FindNode(hcan, addr);
    if (node == NULL)
    {
      continue;
    }

    node->online = 1U;
    node->last_rx_ext_id = header.ExtId;
    node->last_rx_packet = packet;
    node->last_rx_tick = HAL_GetTick();
    node->last_error = HAL_CAN_GetError(hcan);

    if (packet == 0U)
    {
      (void)memset(node->last_rx_stream, 0, sizeof(node->last_rx_stream));
      node->last_rx_stream_len = 0U;
    }

    offset = (uint16_t)((uint16_t)packet * X42S_CAN_MAX_PACKET_DATA);
    if (offset < (uint16_t)sizeof(node->last_rx_stream))
    {
      uint8_t copy_len = header.DLC;
      if ((uint16_t)(offset + copy_len) > (uint16_t)sizeof(node->last_rx_stream))
      {
        copy_len = (uint8_t)((uint16_t)sizeof(node->last_rx_stream) - offset);
      }

      (void)memcpy(&node->last_rx_stream[offset], data, copy_len);
      if ((uint16_t)(offset + copy_len) > node->last_rx_stream_len)
      {
        node->last_rx_stream_len = (uint8_t)(offset + copy_len);
      }
    }

    if (node->last_rx_stream_len > 0U)
    {
      node->last_rx_cmd = node->last_rx_stream[0];
    }
  }
}

void X42sCanBus_OnError(CAN_HandleTypeDef *hcan)
{
  uint32_t index;

  if (hcan == NULL)
  {
    return;
  }

  for (index = 0U; index < (uint32_t)X42S_MOTOR_COUNT; ++index)
  {
    if (g_nodes[index].hcan == hcan)
    {
      g_nodes[index].last_error = HAL_CAN_GetError(hcan);
    }
  }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  X42sCanBus_OnRxPending(hcan);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
  X42sCanBus_OnError(hcan);
}
