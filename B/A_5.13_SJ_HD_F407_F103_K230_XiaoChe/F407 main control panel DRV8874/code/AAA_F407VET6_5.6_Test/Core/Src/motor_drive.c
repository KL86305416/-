#include "motor_drive.h"

#include "tim.h"

typedef struct
{
  TIM_HandleTypeDef *in1_timer;
  uint32_t in1_channel;
  TIM_HandleTypeDef *in2_timer;
  uint32_t in2_channel;
} MotorDriveConfig;

static const MotorDriveConfig g_motor_configs[MOTOR_DRIVE_COUNT] =
{
  {&htim2, TIM_CHANNEL_3, &htim2, TIM_CHANNEL_4},
  {&htim1, TIM_CHANNEL_1, &htim1, TIM_CHANNEL_2},
  {&htim1, TIM_CHANNEL_3, &htim1, TIM_CHANNEL_4},
  {&htim2, TIM_CHANNEL_2, &htim2, TIM_CHANNEL_1}
};

static uint32_t MotorDrive_PercentToCompare(TIM_HandleTypeDef *htim, int16_t percent)
{
  uint32_t auto_reload;
  uint32_t magnitude;

  if (htim == NULL)
  {
    return 0U;
  }

  if (percent < 0)
  {
    percent = (int16_t)(-percent);
  }

  if (percent > 100)
  {
    percent = 100;
  }

  auto_reload = __HAL_TIM_GET_AUTORELOAD(htim);
  magnitude = ((uint32_t)percent * (auto_reload + 1U)) / 100U;
  if (magnitude > auto_reload)
  {
    magnitude = auto_reload;
  }

  return magnitude;
}

static HAL_StatusTypeDef MotorDrive_StartChannel(TIM_HandleTypeDef *htim, uint32_t channel)
{
  if (HAL_TIM_PWM_Start(htim, channel) != HAL_OK)
  {
    return HAL_ERROR;
  }

  __HAL_TIM_SET_COMPARE(htim, channel, 0U);
  return HAL_OK;
}

HAL_StatusTypeDef MotorDrive_Init(void)
{
  uint8_t index;

  for (index = 0U; index < MOTOR_DRIVE_COUNT; ++index)
  {
    if (MotorDrive_StartChannel(g_motor_configs[index].in1_timer,
                                g_motor_configs[index].in1_channel) != HAL_OK)
    {
      return HAL_ERROR;
    }

    if (MotorDrive_StartChannel(g_motor_configs[index].in2_timer,
                                g_motor_configs[index].in2_channel) != HAL_OK)
    {
      return HAL_ERROR;
    }
  }

  MotorDrive_StopAll();
  return HAL_OK;
}

void MotorDrive_StopAll(void)
{
  uint8_t index;

  for (index = 0U; index < MOTOR_DRIVE_COUNT; ++index)
  {
    __HAL_TIM_SET_COMPARE(g_motor_configs[index].in1_timer,
                          g_motor_configs[index].in1_channel,
                          0U);
    __HAL_TIM_SET_COMPARE(g_motor_configs[index].in2_timer,
                          g_motor_configs[index].in2_channel,
                          0U);
  }
}

void MotorDrive_SetPercent(MotorDriveChannel motor, int16_t percent)
{
  const MotorDriveConfig *config;
  uint32_t compare_value;

  if ((motor < MOTOR_DRIVE_A) || (motor >= MOTOR_DRIVE_COUNT))
  {
    return;
  }

  config = &g_motor_configs[motor];

  if (percent > 100)
  {
    percent = 100;
  }
  else if (percent < -100)
  {
    percent = -100;
  }

  compare_value = MotorDrive_PercentToCompare(config->in1_timer, percent);

  if (percent > 0)
  {
    __HAL_TIM_SET_COMPARE(config->in1_timer, config->in1_channel, compare_value);
    __HAL_TIM_SET_COMPARE(config->in2_timer, config->in2_channel, 0U);
  }
  else if (percent < 0)
  {
    __HAL_TIM_SET_COMPARE(config->in1_timer, config->in1_channel, 0U);
    __HAL_TIM_SET_COMPARE(config->in2_timer, config->in2_channel, compare_value);
  }
  else
  {
    __HAL_TIM_SET_COMPARE(config->in1_timer, config->in1_channel, 0U);
    __HAL_TIM_SET_COMPARE(config->in2_timer, config->in2_channel, 0U);
  }
}
