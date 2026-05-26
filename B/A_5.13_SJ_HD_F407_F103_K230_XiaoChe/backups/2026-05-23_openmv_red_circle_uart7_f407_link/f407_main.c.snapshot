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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gray_module.h"
#include "gray_monitor.h"
#include "motor_drive.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
  uint8_t online;
  int16_t x_offset;
  uint16_t radius;
  uint8_t flags;
  uint8_t sequence;
  uint32_t last_update_ms;
  uint8_t rx_index;
  uint8_t rx_buffer[9];
} OpenMvVisionState;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define GRAY_MODULE_UART_TIMEOUT_MS 100U
#define OPENMV_VISION_UART_TIMEOUT_MS 150U
#define VEHICLE_CONTROL_KEY_TEST_SPEED_PERCENT 35
#define OPENMV_VISION_FRAME_LEN 9U
#define OPENMV_VISION_SOF1 0xA5U
#define OPENMV_VISION_SOF2 0x5AU
#define OPENMV_VISION_FLAG_VALID 0x01U
#define OPENMV_VISION_FLAG_RED_FOUND 0x02U
#define OPENMV_VISION_FLAG_CIRCLE_LIKE 0x04U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static GrayModuleState g_gray_module;
static uint8_t g_gray_uart_rx_byte = 0U;
static OpenMvVisionState g_openmv_vision;
static uint8_t g_openmv_uart_rx_byte = 0U;
static GrayMonitorDriveDiag g_drive_diag;
static uint16_t g_left_encoder_last = 0U;
static uint16_t g_right_encoder_last = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void GrayHost_StartUartReceive(void);
static void GrayHost_Poll(void);
static void OpenMvVision_Init(void);
static void OpenMvVision_StartUartReceive(void);
static void OpenMvVision_Poll(void);
static void VehicleControl_Init(void);
static void VehicleControl_StartEncoders(void);
static void VehicleControl_Poll(void);
static uint8_t VehicleControl_IsKeyPressed(GPIO_TypeDef *port, uint16_t pin);
static int16_t VehicleControl_BuildMotorPercent(uint8_t forward_pressed,
                                                uint8_t reverse_pressed);
static void VehicleControl_ApplyMotorOutput(int16_t motor_a_percent,
                                            int16_t motor_b_percent);
static void VehicleControl_RefreshDiagnostics(void);
static void VehicleControl_UpdateLedState(int16_t motor_a_percent,
                                          int16_t motor_b_percent);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint16_t OpenMvVision_ReadU16Le(const uint8_t *data)
{
  return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

static int16_t OpenMvVision_ReadS16Le(const uint8_t *data)
{
  return (int16_t)OpenMvVision_ReadU16Le(data);
}

static void OpenMvVision_RefreshDiagnostics(void)
{
  g_drive_diag.vision_online = g_openmv_vision.online;
  g_drive_diag.vision_x_offset = g_openmv_vision.x_offset;
  g_drive_diag.vision_radius = g_openmv_vision.radius;
  g_drive_diag.vision_flags = g_openmv_vision.flags;
  g_drive_diag.vision_seq = g_openmv_vision.sequence;
}

static uint8_t OpenMvVision_ParseFrame(OpenMvVisionState *state, uint32_t tick_ms)
{
  uint8_t crc;

  if (state == NULL)
  {
    return 0U;
  }

  if ((state->rx_buffer[0] != OPENMV_VISION_SOF1) ||
      (state->rx_buffer[1] != OPENMV_VISION_SOF2))
  {
    return 0U;
  }

  crc = GrayModule_Crc8(&state->rx_buffer[2], 6U);
  if (crc != state->rx_buffer[8])
  {
    return 0U;
  }

  state->x_offset = OpenMvVision_ReadS16Le(&state->rx_buffer[2]);
  state->radius = OpenMvVision_ReadU16Le(&state->rx_buffer[4]);
  state->flags = state->rx_buffer[6];
  state->sequence = state->rx_buffer[7];
  state->online = 1U;
  state->last_update_ms = tick_ms;

  return 1U;
}

static uint8_t OpenMvVision_ConsumeByte(OpenMvVisionState *state,
                                        uint8_t byte,
                                        uint32_t tick_ms)
{
  if (state == NULL)
  {
    return 0U;
  }

  if (state->rx_index == 0U)
  {
    if (byte == OPENMV_VISION_SOF1)
    {
      state->rx_buffer[0] = byte;
      state->rx_index = 1U;
    }

    return 0U;
  }

  if (state->rx_index == 1U)
  {
    if (byte == OPENMV_VISION_SOF2)
    {
      state->rx_buffer[1] = byte;
      state->rx_index = 2U;
    }
    else if (byte == OPENMV_VISION_SOF1)
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

  if (state->rx_index >= OPENMV_VISION_FRAME_LEN)
  {
    state->rx_index = 0U;
    return OpenMvVision_ParseFrame(state, tick_ms);
  }

  return 0U;
}

static void OpenMvVision_Init(void)
{
  g_openmv_vision.online = 0U;
  g_openmv_vision.x_offset = 0;
  g_openmv_vision.radius = 0U;
  g_openmv_vision.flags = 0U;
  g_openmv_vision.sequence = 0U;
  g_openmv_vision.last_update_ms = 0U;
  g_openmv_vision.rx_index = 0U;
}

static uint8_t VehicleControl_IsKeyPressed(GPIO_TypeDef *port, uint16_t pin)
{
  return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET) ? 1U : 0U;
}

static int16_t VehicleControl_BuildMotorPercent(uint8_t forward_pressed,
                                                uint8_t reverse_pressed)
{
  if ((forward_pressed != 0U) && (reverse_pressed == 0U))
  {
    return VEHICLE_CONTROL_KEY_TEST_SPEED_PERCENT;
  }

  if ((forward_pressed == 0U) && (reverse_pressed != 0U))
  {
    return -VEHICLE_CONTROL_KEY_TEST_SPEED_PERCENT;
  }

  return 0;
}

static void VehicleControl_UpdateLedState(int16_t motor_a_percent,
                                          int16_t motor_b_percent)
{
  HAL_GPIO_WritePin(LED_1_GPIO_Port,
                    LED_1_Pin,
                    (motor_a_percent != 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_2_GPIO_Port,
                    LED_2_Pin,
                    (motor_b_percent != 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void VehicleControl_RefreshDiagnostics(void)
{
  uint16_t left_encoder_now;
  uint16_t right_encoder_now;

  g_drive_diag.ain1_state = (HAL_GPIO_ReadPin(DJ_AIN1_GPIO_Port, DJ_AIN1_Pin) == GPIO_PIN_SET) ? 1U : 0U;
  g_drive_diag.ain2_state = (HAL_GPIO_ReadPin(DJ_AIN2_GPIO_Port, DJ_AIN2_Pin) == GPIO_PIN_SET) ? 1U : 0U;
  g_drive_diag.bin1_state = (HAL_GPIO_ReadPin(DJ_BIN1_GPIO_Port, DJ_BIN1_Pin) == GPIO_PIN_SET) ? 1U : 0U;
  g_drive_diag.bin2_state = (HAL_GPIO_ReadPin(DJ_BIN2_GPIO_Port, DJ_BIN2_Pin) == GPIO_PIN_SET) ? 1U : 0U;
  g_drive_diag.pwm_a_compare = (uint16_t)__HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_3);
  g_drive_diag.pwm_b_compare = (uint16_t)__HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_4);

  left_encoder_now = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);
  right_encoder_now = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
  g_drive_diag.left_encoder_delta = (int16_t)(left_encoder_now - g_left_encoder_last);
  g_drive_diag.right_encoder_delta = (int16_t)(right_encoder_now - g_right_encoder_last);
  g_drive_diag.left_encoder_count = left_encoder_now;
  g_drive_diag.right_encoder_count = right_encoder_now;
  g_left_encoder_last = left_encoder_now;
  g_right_encoder_last = right_encoder_now;
}

static void VehicleControl_ApplyMotorOutput(int16_t motor_a_percent,
                                            int16_t motor_b_percent)
{
  g_drive_diag.motor_a_percent = motor_a_percent;
  g_drive_diag.motor_b_percent = motor_b_percent;

  MotorDrive_SetPercent(MOTOR_DRIVE_CHANNEL_A, motor_a_percent);
  MotorDrive_SetPercent(MOTOR_DRIVE_CHANNEL_B, motor_b_percent);
  VehicleControl_UpdateLedState(motor_a_percent, motor_b_percent);
  VehicleControl_RefreshDiagnostics();
}

static void VehicleControl_Init(void)
{
  VehicleControl_ApplyMotorOutput(0, 0);
}

static void VehicleControl_StartEncoders(void)
{
  if (HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL) != HAL_OK)
  {
    Error_Handler();
  }

  g_left_encoder_last = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);
  g_right_encoder_last = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
  g_drive_diag.left_encoder_count = g_left_encoder_last;
  g_drive_diag.right_encoder_count = g_right_encoder_last;
}

static void VehicleControl_Poll(void)
{
  int16_t motor_a_percent;
  int16_t motor_b_percent;

  /* Keys are pulled up in hardware, so a pressed key reads low. */
  g_drive_diag.key_1_pressed = VehicleControl_IsKeyPressed(KEY_1_GPIO_Port, KEY_1_Pin);
  g_drive_diag.key_2_pressed = VehicleControl_IsKeyPressed(KEY_2_GPIO_Port, KEY_2_Pin);
  g_drive_diag.key_3_pressed = VehicleControl_IsKeyPressed(KEY_3_GPIO_Port, KEY_3_Pin);
  g_drive_diag.key_4_pressed = VehicleControl_IsKeyPressed(KEY_4_GPIO_Port, KEY_4_Pin);

  /* Momentary motor self-test:
     KEY1/KEY2 -> motor A forward/reverse
     KEY3/KEY4 -> motor B forward/reverse */
  motor_a_percent = VehicleControl_BuildMotorPercent(g_drive_diag.key_1_pressed,
                                                     g_drive_diag.key_2_pressed);
  motor_b_percent = VehicleControl_BuildMotorPercent(g_drive_diag.key_3_pressed,
                                                     g_drive_diag.key_4_pressed);

  VehicleControl_ApplyMotorOutput(motor_a_percent, motor_b_percent);
}

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
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM12_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_USART3_UART_Init();
  MX_USART6_UART_Init();
  MX_USB_OTG_FS_USB_Init();
  /* USER CODE BEGIN 2 */
  if (MotorDrive_Init() != HAL_OK)
  {
    Error_Handler();
  }
  GrayModule_Init(&g_gray_module);
  OpenMvVision_Init();
  GrayMonitor_Init();
  VehicleControl_StartEncoders();
  VehicleControl_Init();
  GrayHost_StartUartReceive();
  OpenMvVision_StartUartReceive();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    GrayHost_Poll();
    OpenMvVision_Poll();
    VehicleControl_Poll();
    GrayMonitor_Poll(&g_gray_module, &g_drive_diag);
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void GrayHost_StartUartReceive(void)
{
  if (HAL_UART_Receive_IT(&huart6, &g_gray_uart_rx_byte, 1U) != HAL_OK)
  {
    Error_Handler();
  }
}

static void GrayHost_Poll(void)
{
  GrayModule_Service(&g_gray_module, HAL_GetTick(), GRAY_MODULE_UART_TIMEOUT_MS);
}

static void OpenMvVision_StartUartReceive(void)
{
  if (HAL_UART_Receive_IT(&huart4, &g_openmv_uart_rx_byte, 1U) != HAL_OK)
  {
    Error_Handler();
  }
}

static void OpenMvVision_Poll(void)
{
  uint32_t now_ms = HAL_GetTick();

  if ((g_openmv_vision.online != 0U) &&
      ((now_ms - g_openmv_vision.last_update_ms) > OPENMV_VISION_UART_TIMEOUT_MS))
  {
    g_openmv_vision.online = 0U;
  }

  OpenMvVision_RefreshDiagnostics();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if ((huart != NULL) && (huart->Instance == UART4))
  {
    (void)OpenMvVision_ConsumeByte(&g_openmv_vision,
                                   g_openmv_uart_rx_byte,
                                   HAL_GetTick());
    (void)HAL_UART_Receive_IT(&huart4, &g_openmv_uart_rx_byte, 1U);
  }

  if ((huart != NULL) && (huart->Instance == USART6))
  {
    (void)GrayModule_ConsumeRuntimeUartByte(&g_gray_module,
                                            g_gray_uart_rx_byte,
                                            HAL_GetTick());
    (void)HAL_UART_Receive_IT(&huart6, &g_gray_uart_rx_byte, 1U);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if ((huart != NULL) && (huart->Instance == UART4))
  {
    (void)HAL_UART_Receive_IT(&huart4, &g_openmv_uart_rx_byte, 1U);
  }

  if ((huart != NULL) && (huart->Instance == USART6))
  {
    (void)HAL_UART_Receive_IT(&huart6, &g_gray_uart_rx_byte, 1U);
  }
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
