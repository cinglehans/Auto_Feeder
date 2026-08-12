/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../BSP/CRSF/CRSF.h"
#include <string.h>  // 用于标准字符串操作
#include <stdio.h>   // 用于 snprintf 拼接和 sscanf 拆解
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
#define Key1_Pin GPIO_PIN_3
#define Key1_GPIO_Port GPIOE
#define Key0_Pin GPIO_PIN_4
#define Key0_GPIO_Port GPIOE
#define LED1_G_Pin GPIO_PIN_5
#define LED1_G_GPIO_Port GPIOE
#define Key_Up_Pin GPIO_PIN_0
#define Key_Up_GPIO_Port GPIOA
#define Key_Up_EXTI_IRQn EXTI0_IRQn
#define Seal_Detect_Pin GPIO_PIN_5
#define Seal_Detect_GPIO_Port GPIOC
#define Motor_CW_Pin GPIO_PIN_0
#define Motor_CW_GPIO_Port GPIOB
#define Motor_CCW_Pin GPIO_PIN_1
#define Motor_CCW_GPIO_Port GPIOB
#define PWMA_Pin GPIO_PIN_2
#define PWMA_GPIO_Port GPIOB
#define Motor_STBY_Pin GPIO_PIN_11
#define Motor_STBY_GPIO_Port GPIOF
#define RollDirect_Pin GPIO_PIN_12
#define RollDirect_GPIO_Port GPIOF
#define Feed_Detect_Pin GPIO_PIN_14
#define Feed_Detect_GPIO_Port GPIOE
#define PWM_OUT_Pin GPIO_PIN_11
#define PWM_OUT_GPIO_Port GPIOB
#define OLED_CS_Pin GPIO_PIN_9
#define OLED_CS_GPIO_Port GPIOD
#define OLED_RS_Pin GPIO_PIN_11
#define OLED_RS_GPIO_Port GPIOD
#define OLED_RST_Pin GPIO_PIN_13
#define OLED_RST_GPIO_Port GPIOD
#define WIFI_RST_Pin GPIO_PIN_1
#define WIFI_RST_GPIO_Port GPIOD
#define LED0_R_Pin GPIO_PIN_5
#define LED0_R_GPIO_Port GPIOB
#define Beep_Pin GPIO_PIN_8
#define Beep_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
extern crsf_parser_t elrs_parser;
extern volatile uint16_t throttle_raw;
extern volatile uint8_t new_crsf_data;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
