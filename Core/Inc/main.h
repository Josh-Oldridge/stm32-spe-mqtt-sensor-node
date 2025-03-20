/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c in CN0575 SPE Board Project
  * @details        : This file contains common definitions and includes for the CN0575
  *                   Single Pair Ethernet (SPE) board project on the STM32L496ZG-P Nucleo
  *                   board. It defines GPIO pin mappings, constants for standalone ADIN1110
  *                   testing (if USE_LWIP is undefined), and includes necessary HAL and
  *                   FreeRTOS headers. Supports secure MQTT transmission over TLSv1.2 when
  *                   USE_LWIP is defined, or standalone Ethernet frame testing otherwise.
  * @addtogroup main Main Module
  * @{
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
#include "cmsis_os2.h"
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

/** @brief Number of initialization retries for ADIN1110 in standalone mode (5 attempts). */
#define ADIN1110_INIT_ITER  (5)

/** @brief Number of buffer descriptors for standalone frame testing (5 buffers). */
#define BUFF_DESC_COUNT     (5)

/** @brief Maximum frame buffer size including headers and FCS (1532 bytes). */
#define MAX_FRAME_BUF_SIZE  (MAX_FRAME_SIZE + 4 + 2)

/** @brief Total number of frames to send in standalone mode (10 frames). */
#define FRAME_COUNT (10)

/** @brief Number of test frame types in standalone mode (2 types). */
#define TEST_FRAMES_COUNT   (2)

/** @brief Multicast MAC address 0 for standalone mode (filter 1). */
#define MAC_ADDR_0_0  (0x00)
#define MAC_ADDR_0_1  (0xE0)
#define MAC_ADDR_0_2  (0x22)
#define MAC_ADDR_0_3  (0xFE)
#define MAC_ADDR_0_4  (0xDA)
#define MAC_ADDR_0_5  (0xC9)

/** @brief Multicast MAC address 1 for standalone mode (filter 2). */
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
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Reset_Button_Pin GPIO_PIN_13
#define Reset_Button_GPIO_Port GPIOC
#define Reset_Button_EXTI_IRQn EXTI15_10_IRQn
#define Interrupt_Pin GPIO_PIN_12
#define Interrupt_GPIO_Port GPIOF
#define Interrupt_EXTI_IRQn EXTI15_10_IRQn
#define Link_Status_Pin GPIO_PIN_13
#define Link_Status_GPIO_Port GPIOF
#define CN0575_ALERT_LED_Pin GPIO_PIN_9
#define CN0575_ALERT_LED_GPIO_Port GPIOE
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
