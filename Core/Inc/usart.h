/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   LPUART1 Interface for CN0575 Project
  * @details This header declares initialization and buffer management functions for LPUART1 on
  *          the STM32L496ZG-P Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project.
  *          Configures LPUART1 at 115200 baud for debug logging via printf, used across freertos.c,
  *          client_mqtt.c, and boardsupport.c when USE_LWIP is defined, with interrupt-driven RX and
  *          blocking TX capabilities.
  * @addtogroup usart USART Configuration
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef hlpuart1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_LPUART1_UART_Init(void);

/* USER CODE BEGIN Prototypes */
/**
  * @brief Submit a transmit buffer to LPUART1
  * @param [in] buffer Pointer to the transmit buffer
  * @param [in] nbBytes Number of bytes to transmit
  * @return HAL_OK on success, HAL error code on failure
  * @details Queues data for blocking transmission via LPUART1, used by boardsupport.c’s msgWrite.
  */
HAL_StatusTypeDef submitTxBuffer (uint8_t * buffer, int nbBytes);

/**
  * @brief Submit a receive buffer to LPUART1
  * @param [in] buffer Pointer to the receive buffer
  * @param [in] nbBytes Number of bytes to receive
  * @return HAL_OK on success, HAL error code on failure
  * @details Initiates interrupt-driven reception on LPUART1, currently unused in CN0575.
  */
HAL_StatusTypeDef submitRxBuffer (uint8_t * buffer, int nbBytes);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

