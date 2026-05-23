#include "servo.h"

#define SERVO_TIMER               TIM3
#define SERVO_PWM_FREQUENCY_HZ    50U
#define SERVO_TIMER_TICK_HZ       1000000U
#define SERVO_MIN_PULSE_US        500U
#define SERVO_MAX_PULSE_US        2500U

static uint16_t servo_angle_x10[SERVO_CHANNEL_COUNT] = {
  SERVO_CENTER_ANGLE_X10,
  SERVO_CENTER_ANGLE_X10
};

static uint32_t Servo_GetTimerClockHz(void)
{
  uint32_t timer_clock_hz = HAL_RCC_GetPCLK1Freq();

  if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
  {
    timer_clock_hz *= 2U;
  }

  return timer_clock_hz;
}

static uint16_t Servo_AngleX10ToPulseUs(uint16_t angle_x10)
{
  uint32_t pulse_range = SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US;
  uint32_t pulse = SERVO_MIN_PULSE_US +
                   ((uint32_t)angle_x10 * pulse_range) / SERVO_MAX_ANGLE_X10;

  return (uint16_t)pulse;
}

static void Servo_WritePulseUs(Servo_Channel_t channel, uint16_t pulse_us)
{
  if (channel == SERVO_CHANNEL_1)
  {
    SERVO_TIMER->CCR1 = pulse_us;
  }
  else if (channel == SERVO_CHANNEL_2)
  {
    SERVO_TIMER->CCR2 = pulse_us;
  }
}

void Servo_Init(void)
{
  uint32_t timer_clock_hz = Servo_GetTimerClockHz();
  uint32_t prescaler = (timer_clock_hz / SERVO_TIMER_TICK_HZ) - 1U;
  uint32_t period = (SERVO_TIMER_TICK_HZ / SERVO_PWM_FREQUENCY_HZ) - 1U;

  __HAL_RCC_TIM3_CLK_ENABLE();

  SERVO_TIMER->CR1 = 0U;
  SERVO_TIMER->CR2 = 0U;
  SERVO_TIMER->SMCR = 0U;
  SERVO_TIMER->DIER = 0U;
  SERVO_TIMER->CCER = 0U;
  SERVO_TIMER->CCMR1 = 0U;
  SERVO_TIMER->PSC = prescaler;
  SERVO_TIMER->ARR = period;
  SERVO_TIMER->CCR1 = Servo_AngleX10ToPulseUs(servo_angle_x10[SERVO_CHANNEL_1]);
  SERVO_TIMER->CCR2 = Servo_AngleX10ToPulseUs(servo_angle_x10[SERVO_CHANNEL_2]);
  SERVO_TIMER->CNT = 0U;

  SERVO_TIMER->CCMR1 = TIM_CCMR1_OC1PE |
                       TIM_CCMR1_OC1M_1 |
                       TIM_CCMR1_OC1M_2 |
                       TIM_CCMR1_OC2PE |
                       TIM_CCMR1_OC2M_1 |
                       TIM_CCMR1_OC2M_2;
  SERVO_TIMER->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
  SERVO_TIMER->CR1 = TIM_CR1_ARPE;
  SERVO_TIMER->EGR = TIM_EGR_UG;
  SERVO_TIMER->CR1 |= TIM_CR1_CEN;
}

void Servo_SetChannelAngle(Servo_Channel_t channel, uint8_t angle)
{
  Servo_SetChannelAngleX10(channel, (uint16_t)angle * SERVO_ANGLE_SCALE);
}

void Servo_SetChannelAngleX10(Servo_Channel_t channel, uint16_t angle_x10)
{
  if (channel >= SERVO_CHANNEL_COUNT)
  {
    return;
  }

  if (angle_x10 > SERVO_MAX_ANGLE_X10)
  {
    angle_x10 = SERVO_MAX_ANGLE_X10;
  }

  servo_angle_x10[channel] = angle_x10;
  Servo_WritePulseUs(channel, Servo_AngleX10ToPulseUs(servo_angle_x10[channel]));
}

void Servo_CenterChannel(Servo_Channel_t channel)
{
  Servo_SetChannelAngleX10(channel, SERVO_CENTER_ANGLE_X10);
}

uint8_t Servo_GetChannelAngle(Servo_Channel_t channel)
{
  return (uint8_t)((Servo_GetChannelAngleX10(channel) +
                    (SERVO_ANGLE_SCALE / 2U)) / SERVO_ANGLE_SCALE);
}

uint16_t Servo_GetChannelAngleX10(Servo_Channel_t channel)
{
  if (channel >= SERVO_CHANNEL_COUNT)
  {
    return SERVO_CENTER_ANGLE_X10;
  }

  return servo_angle_x10[channel];
}

void Servo_SetAngle(uint8_t angle)
{
  Servo_SetChannelAngle(SERVO_CHANNEL_1, angle);
}

void Servo_SetAngleX10(uint16_t angle_x10)
{
  Servo_SetChannelAngleX10(SERVO_CHANNEL_1, angle_x10);
}

void Servo_MoveUp(void)
{
  uint16_t step_x10 = SERVO_STEP_ANGLE_DEG * SERVO_ANGLE_SCALE;
  uint16_t angle_x10 = servo_angle_x10[SERVO_CHANNEL_1];

  if (angle_x10 > (SERVO_MAX_ANGLE_X10 - step_x10))
  {
    Servo_SetAngleX10(SERVO_MAX_ANGLE_X10);
  }
  else
  {
    Servo_SetAngleX10((uint16_t)(angle_x10 + step_x10));
  }
}

void Servo_MoveDown(void)
{
  uint16_t step_x10 = SERVO_STEP_ANGLE_DEG * SERVO_ANGLE_SCALE;
  uint16_t angle_x10 = servo_angle_x10[SERVO_CHANNEL_1];

  if (angle_x10 < step_x10)
  {
    Servo_SetAngleX10(SERVO_MIN_ANGLE_X10);
  }
  else
  {
    Servo_SetAngleX10((uint16_t)(angle_x10 - step_x10));
  }
}

void Servo_Center(void)
{
  Servo_SetAngle(SERVO_CENTER_ANGLE_DEG);
}

uint8_t Servo_GetAngle(void)
{
  return Servo_GetChannelAngle(SERVO_CHANNEL_1);
}

uint16_t Servo_GetAngleX10(void)
{
  return Servo_GetChannelAngleX10(SERVO_CHANNEL_1);
}
