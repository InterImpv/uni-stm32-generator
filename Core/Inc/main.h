/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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

#include "trigen.h"

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
#define LCDRS_PIN_Pin GPIO_PIN_7
#define LCDRS_PIN_GPIO_Port GPIOE
#define LCDRW_PIN_Pin GPIO_PIN_10
#define LCDRW_PIN_GPIO_Port GPIOE
#define LCDEN_PIN_Pin GPIO_PIN_11
#define LCDEN_PIN_GPIO_Port GPIOE
#define LCDD4_PIN_Pin GPIO_PIN_12
#define LCDD4_PIN_GPIO_Port GPIOE
#define LCDD5_PIN_Pin GPIO_PIN_13
#define LCDD5_PIN_GPIO_Port GPIOE
#define LCDD6_PIN_Pin GPIO_PIN_14
#define LCDD6_PIN_GPIO_Port GPIOE
#define LCDD7_PIN_Pin GPIO_PIN_15
#define LCDD7_PIN_GPIO_Port GPIOE
#define LED_BLUE_Pin GPIO_PIN_15
#define LED_BLUE_GPIO_Port GPIOD
#define TICK_DEC_Pin GPIO_PIN_6
#define TICK_DEC_GPIO_Port GPIOC
#define TICK_DEC_EXTI_IRQn EXTI9_5_IRQn
#define TICK_INC_Pin GPIO_PIN_8
#define TICK_INC_GPIO_Port GPIOC
#define TICK_INC_EXTI_IRQn EXTI9_5_IRQn
#define STEP_DEC_Pin GPIO_PIN_9
#define STEP_DEC_GPIO_Port GPIOC
#define STEP_DEC_EXTI_IRQn EXTI9_5_IRQn
#define STEP_INC_Pin GPIO_PIN_11
#define STEP_INC_GPIO_Port GPIOC
#define STEP_INC_EXTI_IRQn EXTI15_10_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
