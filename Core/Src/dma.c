/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dma.c
  * @brief   DMA Configuration for CN0575 Project
  * @details This file implements DMA initialization for the STM32L496ZG-P Nucleo board in the
  *          CN0575 Single Pair Ethernet (SPE) board project. It enables the DMA1 controller
  *          clock, supporting peripheral-to-memory transfers for ADC1 (sensor data sampling)
  *          and SPI1 (ADIN1110 MAC-PHY communication) as configured in main.c when USE_LWIP is
  *          defined. No additional DMA channels are configured here; specifics are in adc.c
  *          and spi.c.
  * @addtogroup dma DMA Configuration
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
#include "dma.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure DMA                                                              */
/*----------------------------------------------------------------------------*/

/* USER CODE BEGIN 1 */
/**
  * @fn void MX_DMA_Init(void)
  * @brief Enable DMA controller clock
  * @details Initializes the DMA1 controller clock using __HAL_RCC_DMA1_CLK_ENABLE(), called
  *          from main.c before peripheral-specific DMA setups (e.g., ADC1 in adc.c, SPI1 in
  *          spi.c) in the CN0575 project.
  */
/* USER CODE END 1 */

/**
  * Enable DMA controller clock
  */
void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

}

/* USER CODE BEGIN 2 */


/**
  * @}
  */
/* USER CODE END 2 */

