/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   LPUART1 Configuration for CN0575 Project
  * @details This file implements LPUART1 configuration and buffer management for the
  *          STM32L496ZG-P Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project.
  *          Initializes LPUART1 at 115200 baud for debug logging via printf, supporting freertos.c,
  *          client_mqtt.c, and boardsupport.c’s DEBUG_MESSAGE when USE_LWIP is defined. Provides
  *          blocking TX via submitTxBuffer and interrupt-driven RX via submitRxBuffer (currently
  *          unused), with MSP setup for PG7 (TX) and PG8 (RX).
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
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef hlpuart1;

/* LPUART1 init function */

void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */
  /**
    * @fn void MX_LPUART1_UART_Init(void)
	* @brief Initialize LPUART1 for debug logging
	* @details Configures LPUART1 at 115200 baud with 8N1 settings, called from main.c for CN0575 logging.
	*/
  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(uartHandle->Instance==LPUART1)
  {
  /* USER CODE BEGIN LPUART1_MspInit 0 */
  /**
    * @fn void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
	* @brief MSP initialization for LPUART1
	* @param [in] uartHandle UART handle
	* @details Sets up PCLK1 clock, GPIOG pins (PG7 TX, PG8 RX), and enables LPUART1 for debug output.
	*/
  /* USER CODE END LPUART1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* LPUART1 clock enable */
    __HAL_RCC_LPUART1_CLK_ENABLE();

    __HAL_RCC_GPIOG_CLK_ENABLE();
    HAL_PWREx_EnableVddIO2();
    /**LPUART1 GPIO Configuration
    PG7     ------> LPUART1_TX
    PG8     ------> LPUART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* USER CODE BEGIN LPUART1_MspInit 1 */

  /* USER CODE END LPUART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==LPUART1)
  {
  /* USER CODE BEGIN LPUART1_MspDeInit 0 */
  /**
    * @fn void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
	* @brief MSP deinitialization for LPUART1
	* @param [in] uartHandle UART handle
	* @details Disables LPUART1 clock and deinitializes GPIOG pins, cleanup for system shutdown.
	*/
  /* USER CODE END LPUART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPUART1_CLK_DISABLE();

    /**LPUART1 GPIO Configuration
    PG7     ------> LPUART1_TX
    PG8     ------> LPUART1_RX
    */
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_7|GPIO_PIN_8);

  /* USER CODE BEGIN LPUART1_MspDeInit 1 */

  /* USER CODE END LPUART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/**
  * @brief Submit a receive buffer to LPUART1
  * @param [in] buffer Pointer to the receive buffer
  * @param [in] nbBytes Number of bytes to receive
  * @return HAL_OK on success, HAL error code on failure
  * @details Initiates interrupt-driven reception on LPUART1, currently unused as CN0575 relies on
  *          TX-only logging via submitTxBuffer and msgWrite in boardsupport.c.
  */
HAL_StatusTypeDef submitRxBuffer (uint8_t * buffer, int nbBytes)
{

  return HAL_UART_Receive_IT(&hlpuart1, (uint8_t *)buffer, nbBytes);
}

/**
  * @brief Submit a transmit buffer to LPUART1
  * @param [in] buffer Pointer to the transmit buffer
  * @param [in] nbBytes Number of bytes to transmit
  * @return HAL_OK on success, HAL error code on failure
  * @details Queues data for blocking transmission via LPUART1 with a 100ms timeout, used by
  *          boardsupport.c’s msgWrite for debug logging (e.g., DEBUG_MESSAGE output).
  */
HAL_StatusTypeDef submitTxBuffer (uint8_t * buffer, int nbBytes)
{

  return HAL_UART_Transmit(&hlpuart1, (uint8_t*)buffer, nbBytes, 100);
}

/**
  * @brief LPUART1 RX transfer complete callback
  * @param [in] UartHandle UART handle
  * @details Empty callback for interrupt-driven RX completion; unused in CN0575 as RX is not implemented.
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
}

/**
  * @brief LPUART1 error callback
  * @param [in] UartHandle UART handle
  * @details Empty callback for LPUART1 errors; TX errors are handled via HAL return codes in submitTxBuffer.
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
}
/* USER CODE END 1 */
