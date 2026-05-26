#include "motor_drive.h"

#include "tim.h"

typedef struct
{
  TIM_HandleTypeDef *pwm_timer;
  uint32_t pwm_channel;
  GPIO_TypeDef *in1_port;
  uint16_t in1_pin;
  GPIO_TypeDef *in2_port;
  uint16_t in2_pin;
} MotorDriveConfig;

static const MotorDriveConfig g_motor_configs[MOTOR_DRIVE_CHANNEL_COUNT] =
{
  {&htim2, TIM_CHANNEL_3, DJ_AIN1_GPIO_Port, DJ_AIN1_Pin, DJ_AIN2_GPIO_Port, DJ_AIN2_Pin},
  {&htim1, TIM_CHANNEL_4, DJ_BIN1_GPIO_Port, DJ_BIN1_Pin, DJ_BIN2_GPIO_Port, DJ_BIN2_Pin}
};

static void MotorDrive_WriteDirection(const MotorDriveConfig *config,
                                      GPIO_PinState in1_state,
                                      GPIO_PinState in2_state)
{
  HAL_GPIO_WritePin(config->in1_port, config->in1_pin, in1_state);
  HAL_GPIO_WritePin(config->in2_port, config->in2_pin, in2_state);
}

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

static HAL_StatusTypeDef MotorDrive_StartPwmChannel(TIM_HandleTypeDef *htim, uint32_t channel)
{
  if (HAL_TIM_PWM_Start(htim, channel) != HAL_OK)
  {
    return HAL_ERROR;
  }

  __HAL_TIM_SET_COMPARE(htim, channel, 0U);
  return HAL_OK;
}

static void MotorDrive_SetCoast(const MotorDriveConfig *config)
{
  __HAL_TIM_SET_COMPARE(config->pwm_timer, config->pwm_channel, 0U);
  MotorDrive_WriteDirection(config, GPIO_PIN_RESET, GPIO_PIN_RESET);
}

static void MotorDrive_SetBrake(const MotorDriveConfig *config)
{
  __HAL_TIM_SET_COMPARE(config->pwm_timer,
                        config->pwm_channel,
                        __HAL_TIM_GET_AUTORELOAD(config->pwm_timer));
  MotorDrive_WriteDirection(config, GPIO_PIN_SET, GPIO_PIN_SET);
}

HAL_StatusTypeDef MotorDrive_Init(void)
{
  uint8_t index;

  HAL_GPIO_WritePin(DJ_STBY_GPIO_Port, DJ_STBY_Pin, GPIO_PIN_RESET);

  for (index = 0U; index < MOTOR_DRIVE_CHANNEL_COUNT; ++index)
  {
    if (MotorDrive_StartPwmChannel(g_motor_configs[index].pwm_timer,
                                   g_motor_configs[index].pwm_channel) != HAL_OK)
    {
      HAL_GPIO_WritePin(DJ_STBY_GPIO_Port, DJ_STBY_Pin, GPIO_PIN_RESET);
      return HAL_ERROR;
    }

    MotorDrive_SetCoast(&g_motor_configs[index]);
  }

  HAL_GPIO_WritePin(DJ_STBY_GPIO_Port, DJ_STBY_Pin, GPIO_PIN_SET);
  return HAL_OK;
}

void MotorDrive_SetStandby(uint8_t enabled)
{
  if (enabled == 0U)
  {
    MotorDrive_StopAll();
    HAL_GPIO_WritePin(DJ_STBY_GPIO_Port, DJ_STBY_Pin, GPIO_PIN_RESET);
  }
  else
  {
    HAL_GPIO_WritePin(DJ_STBY_GPIO_Port, DJ_STBY_Pin, GPIO_PIN_SET);
  }
}

void MotorDrive_StopAll(void)
{
  uint8_t index;

  for (index = 0U; index < MOTOR_DRIVE_CHANNEL_COUNT; ++index)
  {
    MotorDrive_SetCoast(&g_motor_configs[index]);
  }
}

void MotorDrive_BrakeAll(void)
{
  uint8_t index;

  for (index = 0U; index < MOTOR_DRIVE_CHANNEL_COUNT; ++index)
  {
    MotorDrive_SetBrake(&g_motor_configs[index]);
  }
}

void MotorDrive_SetPercent(MotorDriveChannel motor, int16_t percent)
{
  const MotorDriveConfig *config;
  uint32_t compare_value;

  if ((motor < MOTOR_DRIVE_CHANNEL_A) || (motor >= MOTOR_DRIVE_CHANNEL_COUNT))
  {
    return;
  }

  if (percent > 100)
  {
    percent = 100;
  }
  else if (percent < -100)
  {
    percent = -100;
  }

  config = &g_motor_configs[motor];

  if (percent == 0)
  {
    MotorDrive_SetCoast(config);
    return;
  }

  compare_value = MotorDrive_PercentToCompare(config->pwm_timer, percent);

  if (percent > 0)
  {
    MotorDrive_WriteDirection(config, GPIO_PIN_SET, GPIO_PIN_RESET);
  }
  else
  {
    MotorDrive_WriteDirection(config, GPIO_PIN_RESET, GPIO_PIN_SET);
  }

  __HAL_TIM_SET_COMPARE(config->pwm_timer, config->pwm_channel, compare_value);
}
