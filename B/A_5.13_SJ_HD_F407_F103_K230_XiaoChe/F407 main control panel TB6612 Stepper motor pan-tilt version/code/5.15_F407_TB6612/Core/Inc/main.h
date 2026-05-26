/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Original_Grayscale_4_Pin GPIO_PIN_5
#define Original_Grayscale_4_GPIO_Port GPIOE
#define Original_Grayscale_5_Pin GPIO_PIN_6
#define Original_Grayscale_5_GPIO_Port GPIOE
#define Original_Grayscale_6_Pin GPIO_PIN_13
#define Original_Grayscale_6_GPIO_Port GPIOC
#define Original_Grayscale_7_Pin GPIO_PIN_14
#define Original_Grayscale_7_GPIO_Port GPIOC
#define Original_Grayscale_8_Pin GPIO_PIN_15
#define Original_Grayscale_8_GPIO_Port GPIOC
#define SPI_OLED_RES_Pin GPIO_PIN_2
#define SPI_OLED_RES_GPIO_Port GPIOC
#define SPI_OLED_MOSI_Pin GPIO_PIN_3
#define SPI_OLED_MOSI_GPIO_Port GPIOC
#define OpenMV_RX_Pin GPIO_PIN_0
#define OpenMV_RX_GPIO_Port GPIOA
#define OpenMV_TX_Pin GPIO_PIN_1
#define OpenMV_TX_GPIO_Port GPIOA
#define OLED_DC_Pin GPIO_PIN_4
#define OLED_DC_GPIO_Port GPIOA
#define TFcard_SPI_SCK_Pin GPIO_PIN_5
#define TFcard_SPI_SCK_GPIO_Port GPIOA
#define TFcard_SPI_MISO_Pin GPIO_PIN_6
#define TFcard_SPI_MISO_GPIO_Port GPIOA
#define TFcard_SPI_MOSI_Pin GPIO_PIN_7
#define TFcard_SPI_MOSI_GPIO_Port GPIOA
#define SPI_MicroCD_CS_Pin GPIO_PIN_4
#define SPI_MicroCD_CS_GPIO_Port GPIOC
#define MicroCD_DET_Pin GPIO_PIN_5
#define MicroCD_DET_GPIO_Port GPIOC
#define SPI_OLED_CS_Pin GPIO_PIN_0
#define SPI_OLED_CS_GPIO_Port GPIOB
#define DJ_AIN1_Pin GPIO_PIN_9
#define DJ_AIN1_GPIO_Port GPIOE
#define DJ_AIN2_Pin GPIO_PIN_10
#define DJ_AIN2_GPIO_Port GPIOE
#define DJ_STBY_Pin GPIO_PIN_11
#define DJ_STBY_GPIO_Port GPIOE
#define DJ_BIN1_Pin GPIO_PIN_13
#define DJ_BIN1_GPIO_Port GPIOE
#define DJ_PWM_B_Pin GPIO_PIN_14
#define DJ_PWM_B_GPIO_Port GPIOE
#define I2C_YJ901S_SCL_Pin GPIO_PIN_10
#define I2C_YJ901S_SCL_GPIO_Port GPIOB
#define I2C_YJ901S_SDA_Pin GPIO_PIN_11
#define I2C_YJ901S_SDA_GPIO_Port GPIOB
#define SPI_OLED_SCK_Pin GPIO_PIN_13
#define SPI_OLED_SCK_GPIO_Port GPIOB
#define PWM_DuoJi_A_Pin GPIO_PIN_14
#define PWM_DuoJi_A_GPIO_Port GPIOB
#define PWM_DuoJi_B_Pin GPIO_PIN_15
#define PWM_DuoJi_B_GPIO_Port GPIOB
#define USART_YJ901S_RX_Pin GPIO_PIN_8
#define USART_YJ901S_RX_GPIO_Port GPIOD
#define USART_YJ901S_TX_Pin GPIO_PIN_9
#define USART_YJ901S_TX_GPIO_Port GPIOD
#define DJ_BM_A_1_Pin GPIO_PIN_12
#define DJ_BM_A_1_GPIO_Port GPIOD
#define DJ_BM_A_2_Pin GPIO_PIN_13
#define DJ_BM_A_2_GPIO_Port GPIOD
#define USART_HD_RX_Pin GPIO_PIN_6
#define USART_HD_RX_GPIO_Port GPIOC
#define USART_HD_TX_Pin GPIO_PIN_7
#define USART_HD_TX_GPIO_Port GPIOC
#define I2C_HD_SDA_Pin GPIO_PIN_9
#define I2C_HD_SDA_GPIO_Port GPIOC
#define I2C_HD_SCL_Pin GPIO_PIN_8
#define I2C_HD_SCL_GPIO_Port GPIOA
#define DJ_BIN2_Pin GPIO_PIN_15
#define DJ_BIN2_GPIO_Port GPIOA
#define KEY_1_Pin GPIO_PIN_3
#define KEY_1_GPIO_Port GPIOD
#define KEY_2_Pin GPIO_PIN_4
#define KEY_2_GPIO_Port GPIOD
#define KEY_3_Pin GPIO_PIN_5
#define KEY_3_GPIO_Port GPIOD
#define KEY_4_Pin GPIO_PIN_6
#define KEY_4_GPIO_Port GPIOD
#define DJ_PWM_PWM_A_Pin GPIO_PIN_3
#define DJ_PWM_PWM_A_GPIO_Port GPIOB
#define DJ_BM_B_1_Pin GPIO_PIN_4
#define DJ_BM_B_1_GPIO_Port GPIOB
#define DJ_BM_B_2_Pin GPIO_PIN_5
#define DJ_BM_B_2_GPIO_Port GPIOB
#define I2C_OLED_SDA_Pin GPIO_PIN_7
#define I2C_OLED_SDA_GPIO_Port GPIOB
#define LED_1_Pin GPIO_PIN_9
#define LED_1_GPIO_Port GPIOB
#define LED_2_Pin GPIO_PIN_0
#define LED_2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
