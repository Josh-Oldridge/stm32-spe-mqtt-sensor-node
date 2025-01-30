/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#ifdef USE_LWIP
  /* When using lwIP, these definitions come from lwIP_adin1110_app.c */
#else /* USE_LWIP not defined */
#define ADIN1110_INIT_ITER  (5)
#define BUFF_DESC_COUNT     (5)
#define MAX_FRAME_BUF_SIZE  (MAX_FRAME_SIZE + 4 + 2)

#define FRAME_COUNT (10)
#define MAX_FRAME_BUF_SIZE  (MAX_FRAME_SIZE + 4 + 2)
#define TEST_FRAMES_COUNT   (2)

#define MAC_ADDR_0_0  (0x00)
#define MAC_ADDR_0_1  (0xE0)
#define MAC_ADDR_0_2  (0x22)
#define MAC_ADDR_0_3  (0xFE)
#define MAC_ADDR_0_4  (0xDA)
#define MAC_ADDR_0_5  (0xC9)

#define MAC_ADDR_1_0  (0x00)
#define MAC_ADDR_1_1  (0xE0)
#define MAC_ADDR_1_2  (0x22)
#define MAC_ADDR_1_3  (0xFE)
#define MAC_ADDR_1_4  (0xDA)
#define MAC_ADDR_1_5  (0xCA)
#endif
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define DHCP_CHECK_INTERVAL_MS 500
#define DHCP_WAIT_TIMEOUT_MS 30000
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void SystemClock_Config(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Reset_Button_Pin GPIO_PIN_13
#define Reset_Button_GPIO_Port GPIOC
#define Reset_Button_EXTI_IRQn EXTI15_10_IRQn
#define TS_TIMER_MS_SEL_Pin GPIO_PIN_3
#define TS_TIMER_MS_SEL_GPIO_Port GPIOA
#define Interrupt_Pin GPIO_PIN_12
#define Interrupt_GPIO_Port GPIOF
#define Interrupt_EXTI_IRQn EXTI15_10_IRQn
#define Link_Status_Pin GPIO_PIN_13
#define Link_Status_GPIO_Port GPIOF
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define SPI1_CS_Pin GPIO_PIN_14
#define SPI1_CS_GPIO_Port GPIOD
#define Reset_Pin GPIO_PIN_15
#define Reset_GPIO_Port GPIOD
#define LD1_Pin GPIO_PIN_7
#define LD1_GPIO_Port GPIOC
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
