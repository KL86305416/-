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
#define Orgial_HD_1_Pin GPIO_PIN_2
#define Orgial_HD_1_GPIO_Port GPIOE
#define Orgial_HD_2_Pin GPIO_PIN_3
#define Orgial_HD_2_GPIO_Port GPIOE
#define Orgial_HD_3_Pin GPIO_PIN_4
#define Orgial_HD_3_GPIO_Port GPIOE
#define Orgial_HD_4_Pin GPIO_PIN_5
#define Orgial_HD_4_GPIO_Port GPIOE
#define Orgial_HD_5_Pin GPIO_PIN_6
#define Orgial_HD_5_GPIO_Port GPIOE
#define Orgial_HD_6_Pin GPIO_PIN_13
#define Orgial_HD_6_GPIO_Port GPIOC
#define Orgial_HD_7_Pin GPIO_PIN_0
#define Orgial_HD_7_GPIO_Port GPIOC
#define Orgial_HD_8_Pin GPIO_PIN_1
#define Orgial_HD_8_GPIO_Port GPIOC
#define OLED_RES_Pin GPIO_PIN_2
#define OLED_RES_GPIO_Port GPIOC
#define OLED2_MOSI_Pin GPIO_PIN_3
#define OLED2_MOSI_GPIO_Port GPIOC
#define SJ_TX_Pin GPIO_PIN_0
#define SJ_TX_GPIO_Port GPIOA
#define SJ_RX_Pin GPIO_PIN_1
#define SJ_RX_GPIO_Port GPIOA
#define DJ_A_IN1_Pin GPIO_PIN_2
#define DJ_A_IN1_GPIO_Port GPIOA
#define DJ_A_IN2_Pin GPIO_PIN_3
#define DJ_A_IN2_GPIO_Port GPIOA
#define OLED_DC_Pin GPIO_PIN_4
#define OLED_DC_GPIO_Port GPIOA
#define TF_SCK_Pin GPIO_PIN_5
#define TF_SCK_GPIO_Port GPIOA
#define TF_MISO_Pin GPIO_PIN_6
#define TF_MISO_GPIO_Port GPIOA
#define TF_MOSI_Pin GPIO_PIN_7
#define TF_MOSI_GPIO_Port GPIOA
#define SPI_MicroCD_CS_Pin GPIO_PIN_4
#define SPI_MicroCD_CS_GPIO_Port GPIOC
#define MicroCD_DET_Pin GPIO_PIN_5
#define MicroCD_DET_GPIO_Port GPIOC
#define OLED_CS_Pin GPIO_PIN_0
#define OLED_CS_GPIO_Port GPIOB
#define DJ_B_IN1_Pin GPIO_PIN_9
#define DJ_B_IN1_GPIO_Port GPIOE
#define DJ_B_IN2_Pin GPIO_PIN_11
#define DJ_B_IN2_GPIO_Port GPIOE
#define DJ_C_IN1_Pin GPIO_PIN_13
#define DJ_C_IN1_GPIO_Port GPIOE
#define DJ_C_IN2_Pin GPIO_PIN_14
#define DJ_C_IN2_GPIO_Port GPIOE
#define YJ901S_SCL_Pin GPIO_PIN_10
#define YJ901S_SCL_GPIO_Port GPIOB
#define YJ901S_SDA_Pin GPIO_PIN_11
#define YJ901S_SDA_GPIO_Port GPIOB
#define OLED2_SCK_Pin GPIO_PIN_13
#define OLED2_SCK_GPIO_Port GPIOB
#define PWM_DuoJi_A_Pin GPIO_PIN_14
#define PWM_DuoJi_A_GPIO_Port GPIOB
#define PWM_DuoJi_B_Pin GPIO_PIN_15
#define PWM_DuoJi_B_GPIO_Port GPIOB
#define YJ901S_TX_Pin GPIO_PIN_8
#define YJ901S_TX_GPIO_Port GPIOD
#define YJ901S_RX_Pin GPIO_PIN_9
#define YJ901S_RX_GPIO_Port GPIOD
#define DJ_BM_A_1_Pin GPIO_PIN_12
#define DJ_BM_A_1_GPIO_Port GPIOD
#define DJ_BM_A_2_Pin GPIO_PIN_13
#define DJ_BM_A_2_GPIO_Port GPIOD
#define CK_HD_TX_Pin GPIO_PIN_6
#define CK_HD_TX_GPIO_Port GPIOC
#define CK_HD_RX_Pin GPIO_PIN_7
#define CK_HD_RX_GPIO_Port GPIOC
#define HD_SDA_Pin GPIO_PIN_9
#define HD_SDA_GPIO_Port GPIOC
#define HD_SCL_Pin GPIO_PIN_8
#define HD_SCL_GPIO_Port GPIOA
#define DJ_D_IN2_Pin GPIO_PIN_15
#define DJ_D_IN2_GPIO_Port GPIOA
#define CK_Resrve_TX_Pin GPIO_PIN_12
#define CK_Resrve_TX_GPIO_Port GPIOC
#define CK_Resrve_RX_Pin GPIO_PIN_2
#define CK_Resrve_RX_GPIO_Port GPIOD
#define KEY_1_Pin GPIO_PIN_3
#define KEY_1_GPIO_Port GPIOD
#define KEY_2_Pin GPIO_PIN_4
#define KEY_2_GPIO_Port GPIOD
#define KEY_3_Pin GPIO_PIN_5
#define KEY_3_GPIO_Port GPIOD
#define KEY_4_Pin GPIO_PIN_6
#define KEY_4_GPIO_Port GPIOD
#define DJ_D_IN1_Pin GPIO_PIN_3
#define DJ_D_IN1_GPIO_Port GPIOB
#define DJ_BM_B_1_Pin GPIO_PIN_4
#define DJ_BM_B_1_GPIO_Port GPIOB
#define DJ_BM_B_2_Pin GPIO_PIN_5
#define DJ_BM_B_2_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_7
#define OLED_SDA_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_8
#define OLED_SCL_GPIO_Port GPIOB
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
