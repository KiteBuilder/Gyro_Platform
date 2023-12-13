/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#define TFT_DC_Pin GPIO_PIN_8
#define TFT_DC_GPIO_Port GPIOA
#define TFT_RST_Pin GPIO_PIN_9
#define TFT_RST_GPIO_Port GPIOA
#define TFT_CS_Pin GPIO_PIN_10
#define TFT_CS_GPIO_Port GPIOA
#define TFT_LED_Pin GPIO_PIN_11
#define TFT_LED_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

// cleanup BASEPRI restore function, with global memory barrier
static inline void __basepriRestoreMem(uint8_t *val)
{
    __set_BASEPRI(*val);
}

// set BASEPRI_MAX function, with global memory barrier, returns true
static inline uint8_t __basepriSetMemRetVal(uint8_t prio)
{
    __set_BASEPRI_MAX(prio);
    return 1;
}

#define NVIC_PRIO_MAX 1
#define ATOMIC_BLOCK(prio) for ( uint8_t __basepri_save __attribute__((__cleanup__(__basepriRestoreMem))) = __get_BASEPRI(), \
                                     __ToDo = __basepriSetMemRetVal((prio) << (8U - __NVIC_PRIO_BITS)); __ToDo ; __ToDo = 0 )

typedef uint32_t timeMs_t;
typedef uint64_t timeUs_t;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
