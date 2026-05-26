#ifndef __X42S_CAN_BUS_H__
#define __X42S_CAN_BUS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define X42S_MOTOR1_ADDR               1U
#define X42S_MOTOR2_ADDR               2U
#define X42S_CAN_MAX_STREAM_BYTES      24U
#define X42S_MOTOR_OFFLINE_TIMEOUT_MS  500U

typedef enum
{
  X42S_PROTOCOL_EMM = 0,
  X42S_PROTOCOL_X = 1
} X42sProtocolTypeDef;

#define X42S_DEFAULT_PROTOCOL X42S_PROTOCOL_EMM

typedef enum
{
  X42S_MOTOR_1 = 0,
  X42S_MOTOR_2 = 1,
  X42S_MOTOR_COUNT
} X42sMotorId;

typedef struct
{
  CAN_HandleTypeDef *hcan;
  uint8_t bus_id;
  uint8_t motor_addr;
  uint8_t online;
  uint8_t last_tx_cmd;
  uint8_t last_tx_packet_count;
  uint8_t last_tx_stream_len;
  uint8_t last_tx_stream[X42S_CAN_MAX_STREAM_BYTES];
  uint8_t last_rx_cmd;
  uint8_t last_rx_packet;
  uint8_t last_rx_stream_len;
  uint8_t last_rx_stream[X42S_CAN_MAX_STREAM_BYTES];
  uint32_t last_tx_ext_id;
  uint32_t last_rx_ext_id;
  uint32_t last_tx_tick;
  uint32_t last_rx_tick;
  uint32_t last_error;
} X42sMotorNode;

HAL_StatusTypeDef X42sCanBus_Init(void);
void X42sCanBus_Poll(uint32_t now_ms);

const X42sMotorNode *X42sCanBus_GetMotorNode(X42sMotorId motor_id);
X42sProtocolTypeDef X42sCanBus_GetProtocol(void);
const char *X42sCanBus_GetProtocolName(void);

HAL_StatusTypeDef X42sCanBus_ReadVersion(X42sMotorId motor_id);
HAL_StatusTypeDef X42sCanBus_ReadSpeed(X42sMotorId motor_id);
HAL_StatusTypeDef X42sCanBus_ReadPosition(X42sMotorId motor_id);
HAL_StatusTypeDef X42sCanBus_ReadStatus(X42sMotorId motor_id);
HAL_StatusTypeDef X42sCanBus_ReadHomingStatus(X42sMotorId motor_id);
HAL_StatusTypeDef X42sCanBus_Enable(X42sMotorId motor_id, uint8_t enable, uint8_t sync);
HAL_StatusTypeDef X42sCanBus_Stop(X42sMotorId motor_id, uint8_t sync);
HAL_StatusTypeDef X42sCanBus_TriggerSync(void);
HAL_StatusTypeDef X42sCanBus_SetVelocity(X42sMotorId motor_id,
                                         uint8_t direction,
                                         uint16_t velocity,
                                         uint16_t ramp,
                                         uint8_t sync);
HAL_StatusTypeDef X42sCanBus_SetPosition(X42sMotorId motor_id,
                                         uint8_t direction,
                                         uint16_t velocity,
                                         uint16_t accel,
                                         uint16_t decel,
                                         int32_t target,
                                         uint8_t mode,
                                         uint8_t sync);

void X42sCanBus_OnRxPending(CAN_HandleTypeDef *hcan);
void X42sCanBus_OnError(CAN_HandleTypeDef *hcan);

#ifdef __cplusplus
}
#endif

#endif /* __X42S_CAN_BUS_H__ */
