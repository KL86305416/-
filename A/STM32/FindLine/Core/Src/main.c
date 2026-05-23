/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gray_sensor.h"
#include "line_follower.h"
#include "line_follower_config.h"
#include "oled.h"
#include "servo.h"
#include "stm32g431xx.h"
#include "stm32g4xx_hal_gpio.h"
#include "tb6612.h"

#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  APP_SCREEN_MENU = 0,
  APP_SCREEN_LINE_FOLLOWER = 1,
  APP_SCREEN_SERVO_DEBUG = 2
} App_Screen_t;

typedef enum
{
  APP_BUTTON_UP = 0,
  APP_BUTTON_DOWN,
  APP_BUTTON_OK,
  APP_BUTTON_BACK,
  APP_BUTTON_COUNT
} App_ButtonId_t;

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  GPIO_PinState stable_state;
  GPIO_PinState sample_state;
  uint32_t sample_tick;
} App_Button_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_BUTTON_ACTIVE_STATE  GPIO_PIN_RESET
#define APP_BUTTON_DEBOUNCE_MS   35U
#define APP_MENU_ITEM_COUNT      2U
#define APP_SERVO_CONTROL_PERIOD_MS       2U
#define APP_SERVO_SPEED_DEG_X10_PER_SEC   3000U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t oled_ready = 0U;
static uint32_t oled_last_update_tick = 0U;
static App_Screen_t app_screen = APP_SCREEN_MENU;
static uint8_t menu_selected_index = 0U;
static uint8_t menu_dirty = 1U;
static uint32_t servo_last_move_tick[SERVO_CHANNEL_COUNT] = {0U, 0U};
static int8_t servo_move_direction[SERVO_CHANNEL_COUNT] = {0, 0};
static uint8_t servo_ignore_buttons_until_release = 0U;
static App_Button_t app_buttons[APP_BUTTON_COUNT] = {
  {GPIOC, GPIO_PIN_3, GPIO_PIN_SET, GPIO_PIN_SET, 0U},
  {GPIOC, GPIO_PIN_2, GPIO_PIN_SET, GPIO_PIN_SET, 0U},
  {GPIOC, GPIO_PIN_1, GPIO_PIN_SET, GPIO_PIN_SET, 0U},
  {GPIOC, GPIO_PIN_0, GPIO_PIN_SET, GPIO_PIN_SET, 0U}
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void App_ButtonInit(void);
static uint8_t App_ButtonScan(void);
static uint8_t App_ButtonPressedMask(void);
static void App_EnterMenu(void);
static void App_EnterLineFollower(void);
static void App_EnterServoDebug(void);
static void App_HandleMenuButtons(uint8_t button_events);
static void App_ShowMenuScreen(void);
static void App_ShowLineStatus(const LineFollower_Status_t *status);
static void App_UpdateServoDebug(uint8_t button_events);
static void App_UpdateServoMotion(Servo_Channel_t channel,
                                  int8_t direction,
                                  uint32_t now);
static void App_ShowServoDebugScreen(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  App_ButtonInit();
  GraySensor_Init();
  TB6612_Init();
  LineFollower_Init();
  Servo_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  if (OLED_Init() == HAL_OK)
  {
    oled_ready = 1U;
    App_ShowMenuScreen();
  }
  oled_last_update_tick = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint32_t loop_delay_ms = LINE_FOLLOWER_CONTROL_PERIOD_MS;
    uint8_t button_events = App_ButtonScan();

    if (app_screen == APP_SCREEN_MENU)
    {
      App_HandleMenuButtons(button_events);

      if ((oled_ready != 0U) && (menu_dirty != 0U))
      {
        App_ShowMenuScreen();
        oled_last_update_tick = HAL_GetTick();
      }
    }
    else
    {
      if (((button_events & (1U << APP_BUTTON_BACK)) != 0U) &&
          (app_screen != APP_SCREEN_SERVO_DEBUG))
      {
        App_EnterMenu();
      }
      else if (app_screen == APP_SCREEN_LINE_FOLLOWER)
      {
        LineFollower_Status_t line_status;

        line_status = LineFollower_Update();

        if ((oled_ready != 0U) &&
            ((HAL_GetTick() - oled_last_update_tick) >= LINE_FOLLOWER_OLED_UPDATE_MS))
        {
          App_ShowLineStatus(&line_status);
          oled_last_update_tick = HAL_GetTick();
        }
      }
      else if (app_screen == APP_SCREEN_SERVO_DEBUG)
      {
        App_UpdateServoDebug(button_events);
        loop_delay_ms = APP_SERVO_CONTROL_PERIOD_MS;
      }
    }

    HAL_Delay(loop_delay_ms);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void App_ButtonInit(void)
{
  uint32_t now = HAL_GetTick();

  for (uint8_t i = 0U; i < APP_BUTTON_COUNT; i++)
  {
    GPIO_PinState state = HAL_GPIO_ReadPin(app_buttons[i].port, app_buttons[i].pin);

    app_buttons[i].stable_state = state;
    app_buttons[i].sample_state = state;
    app_buttons[i].sample_tick = now;
  }
}

static uint8_t App_ButtonScan(void)
{
  uint8_t events = 0U;
  uint32_t now = HAL_GetTick();

  for (uint8_t i = 0U; i < APP_BUTTON_COUNT; i++)
  {
    GPIO_PinState sample = HAL_GPIO_ReadPin(app_buttons[i].port, app_buttons[i].pin);

    if (sample != app_buttons[i].sample_state)
    {
      app_buttons[i].sample_state = sample;
      app_buttons[i].sample_tick = now;
    }
    else if (((now - app_buttons[i].sample_tick) >= APP_BUTTON_DEBOUNCE_MS) &&
             (sample != app_buttons[i].stable_state))
    {
      app_buttons[i].stable_state = sample;
      if (sample == APP_BUTTON_ACTIVE_STATE)
      {
        events |= (uint8_t)(1U << i);
      }
    }
  }

  return events;
}

static uint8_t App_ButtonPressedMask(void)
{
  uint8_t pressed_mask = 0U;

  for (uint8_t i = 0U; i < APP_BUTTON_COUNT; i++)
  {
    if (app_buttons[i].stable_state == APP_BUTTON_ACTIVE_STATE)
    {
      pressed_mask |= (uint8_t)(1U << i);
    }
  }

  return pressed_mask;
}

static void App_EnterMenu(void)
{
  LineFollower_Stop();
  app_screen = APP_SCREEN_MENU;
  menu_dirty = 1U;
}

static void App_EnterLineFollower(void)
{
  LineFollower_Init();
  app_screen = APP_SCREEN_LINE_FOLLOWER;
  oled_last_update_tick = 0U;
}

static void App_EnterServoDebug(void)
{
  uint32_t now = HAL_GetTick();

  Servo_CenterChannel(SERVO_CHANNEL_1);
  Servo_CenterChannel(SERVO_CHANNEL_2);
  servo_last_move_tick[SERVO_CHANNEL_1] = now;
  servo_last_move_tick[SERVO_CHANNEL_2] = now;
  servo_move_direction[SERVO_CHANNEL_1] = 0;
  servo_move_direction[SERVO_CHANNEL_2] = 0;
  servo_ignore_buttons_until_release = App_ButtonPressedMask();
  app_screen = APP_SCREEN_SERVO_DEBUG;
  oled_last_update_tick = 0U;

  if (oled_ready != 0U)
  {
    App_ShowServoDebugScreen();
    oled_last_update_tick = now;
  }
}

static void App_HandleMenuButtons(uint8_t button_events)
{
  if (((button_events & (1U << APP_BUTTON_UP)) != 0U) && (APP_MENU_ITEM_COUNT > 1U))
  {
    if (menu_selected_index == 0U)
    {
      menu_selected_index = APP_MENU_ITEM_COUNT - 1U;
    }
    else
    {
      menu_selected_index--;
    }
    menu_dirty = 1U;
  }

  if (((button_events & (1U << APP_BUTTON_DOWN)) != 0U) && (APP_MENU_ITEM_COUNT > 1U))
  {
    menu_selected_index++;
    if (menu_selected_index >= APP_MENU_ITEM_COUNT)
    {
      menu_selected_index = 0U;
    }
    menu_dirty = 1U;
  }

  if ((button_events & (1U << APP_BUTTON_OK)) != 0U)
  {
    if (menu_selected_index == 0U)
    {
      App_EnterLineFollower();
    }
    else if (menu_selected_index == 1U)
    {
      App_EnterServoDebug();
    }
  }
}

static void App_ShowMenuScreen(void)
{
  char line[22];

  OLED_Clear();
  OLED_SetCursor(0, 0);
  OLED_WriteString("Main Menu", OLED_COLOR_WHITE);

  (void)snprintf(line, sizeof(line), "%c Line Follow",
                 (menu_selected_index == 0U) ? '>' : ' ');
  OLED_SetCursor(0, 16);
  OLED_WriteString(line, OLED_COLOR_WHITE);

  (void)snprintf(line, sizeof(line), "%c Servo Debug",
                 (menu_selected_index == 1U) ? '>' : ' ');
  OLED_SetCursor(0, 24);
  OLED_WriteString(line, OLED_COLOR_WHITE);

  (void)OLED_UpdateScreen();

  menu_dirty = 0U;
}

static void App_ShowLineStatus(const LineFollower_Status_t *status)
{
  char line[22];

  OLED_Clear();
  OLED_SetCursor(0, 0);
  if (status->line_lost != 0U)
  {
    OLED_WriteString("LINE: LOST", OLED_COLOR_WHITE);
  }
  else
  {
    OLED_WriteString((status->curve_active != 0U) ? "LINE: CURVE" : "LINE: TRACK",
                     OLED_COLOR_WHITE);
  }

  (void)snprintf(line, sizeof(line), "RAW:0x%02X CNT:%u",
                 (unsigned int)status->raw_value,
                 (unsigned int)status->active_count);
  OLED_SetCursor(0, 8);
  OLED_WriteString(line, OLED_COLOR_WHITE);

  (void)snprintf(line, sizeof(line), "ERR:%4d", status->error_x100);
  OLED_SetCursor(0, 16);
  OLED_WriteString(line, OLED_COLOR_WHITE);

  (void)snprintf(line, sizeof(line), "L:%4d R:%4d",
                 status->left_speed,
                 status->right_speed);
  OLED_SetCursor(0, 24);
  OLED_WriteString(line, OLED_COLOR_WHITE);

  (void)OLED_UpdateScreen();
}

static void App_UpdateServoDebug(uint8_t button_events)
{
  uint8_t pressed_buttons = App_ButtonPressedMask();
  uint32_t now = HAL_GetTick();
  uint8_t active_buttons;
  int8_t servo1_direction = 0;
  int8_t servo2_direction = 0;

  (void)button_events;

  servo_ignore_buttons_until_release &= pressed_buttons;
  active_buttons = (uint8_t)(pressed_buttons & ~servo_ignore_buttons_until_release);

  if (((active_buttons & (1U << APP_BUTTON_UP)) != 0U) &&
      ((active_buttons & (1U << APP_BUTTON_DOWN)) == 0U))
  {
    servo1_direction = 1;
  }
  else if (((active_buttons & (1U << APP_BUTTON_DOWN)) != 0U) &&
           ((active_buttons & (1U << APP_BUTTON_UP)) == 0U))
  {
    servo1_direction = -1;
  }

  if (((active_buttons & (1U << APP_BUTTON_OK)) != 0U) &&
      ((active_buttons & (1U << APP_BUTTON_BACK)) == 0U))
  {
    servo2_direction = 1;
  }
  else if (((active_buttons & (1U << APP_BUTTON_BACK)) != 0U) &&
           ((active_buttons & (1U << APP_BUTTON_OK)) == 0U))
  {
    servo2_direction = -1;
  }

  App_UpdateServoMotion(SERVO_CHANNEL_1, servo1_direction, now);
  App_UpdateServoMotion(SERVO_CHANNEL_2, servo2_direction, now);
}

static void App_UpdateServoMotion(Servo_Channel_t channel,
                                  int8_t direction,
                                  uint32_t now)
{
  if (channel >= SERVO_CHANNEL_COUNT)
  {
    return;
  }

  if (direction == 0)
  {
    servo_last_move_tick[channel] = now;
    servo_move_direction[channel] = 0;
  }
  else if (direction != servo_move_direction[channel])
  {
    servo_last_move_tick[channel] = now;
    servo_move_direction[channel] = direction;
  }
  else
  {
    uint32_t elapsed_ms = now - servo_last_move_tick[channel];

    if (elapsed_ms != 0U)
    {
      uint32_t delta_x10 = (APP_SERVO_SPEED_DEG_X10_PER_SEC * elapsed_ms) / 1000U;

      if (delta_x10 == 0U)
      {
        delta_x10 = 1U;
      }

      if (direction > 0)
      {
        uint16_t angle_x10 = Servo_GetChannelAngleX10(channel);

        if (angle_x10 >= SERVO_MAX_ANGLE_X10)
        {
          Servo_SetChannelAngleX10(channel, SERVO_MAX_ANGLE_X10);
        }
        else if (delta_x10 >= (uint32_t)(SERVO_MAX_ANGLE_X10 - angle_x10))
        {
          Servo_SetChannelAngleX10(channel, SERVO_MAX_ANGLE_X10);
        }
        else
        {
          Servo_SetChannelAngleX10(channel, (uint16_t)(angle_x10 + delta_x10));
        }
      }
      else
      {
        uint16_t angle_x10 = Servo_GetChannelAngleX10(channel);

        if (angle_x10 <= SERVO_MIN_ANGLE_X10)
        {
          Servo_SetChannelAngleX10(channel, SERVO_MIN_ANGLE_X10);
        }
        else if (delta_x10 >= angle_x10)
        {
          Servo_SetChannelAngleX10(channel, SERVO_MIN_ANGLE_X10);
        }
        else
        {
          Servo_SetChannelAngleX10(channel, (uint16_t)(angle_x10 - delta_x10));
        }
      }

      servo_last_move_tick[channel] = now;
    }
  }
}

static void App_ShowServoDebugScreen(void)
{
  char line[22];
  uint16_t servo1_angle_x10 = Servo_GetChannelAngleX10(SERVO_CHANNEL_1);
  uint16_t servo2_angle_x10 = Servo_GetChannelAngleX10(SERVO_CHANNEL_2);

  OLED_Clear();
  OLED_SetCursor(0, 0);
  OLED_WriteString("Dual Servo Debug", OLED_COLOR_WHITE);

  OLED_SetCursor(0, 8);
  OLED_WriteString("S1 PA6  S2 PA7", OLED_COLOR_WHITE);

  (void)snprintf(line, sizeof(line), "S1:%3u.%u S2:%3u.%u",
                 (unsigned int)(servo1_angle_x10 / SERVO_ANGLE_SCALE),
                 (unsigned int)(servo1_angle_x10 % SERVO_ANGLE_SCALE),
                 (unsigned int)(servo2_angle_x10 / SERVO_ANGLE_SCALE),
                 (unsigned int)(servo2_angle_x10 % SERVO_ANGLE_SCALE));
  OLED_SetCursor(0, 16);
  OLED_WriteString(line, OLED_COLOR_WHITE);

  (void)OLED_UpdateScreen();
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
