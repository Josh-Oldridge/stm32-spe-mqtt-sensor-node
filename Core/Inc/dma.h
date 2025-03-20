/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dma.h
  * @brief   DMA Interface for CN0575 Project
  * @details This header declares the DMA initialization function for the STM32L496ZG-P Nucleo
  *          board in the CN0575 Single Pair Ethernet (SPE) board project. It enables DMA1 clock
  *          configuration for peripheral-to-memory transfers, specifically used for ADC1 (sensor
  *          data sampling) and SPI1 (ADIN1110 communication) in main.c when USE_LWIP is defined.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DMA_H__
#define __DMA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* DMA memory to memory transfer handles -------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_DMA_Init(void);

/* USER CODE BEGIN Prototypes */
/**
  * @fn void MX_DMA_Init(void)
  * @brief Initialize DMA controller for CN0575 peripherals
  * @details Configures the DMA1 controller clock, enabling transfers for ADC1 and SPI1 as used
  *          in main.c for sensor data and ADIN1110 communication in the CN0575 project.
  */

/**
  * @}
  */
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __DMA_H__ */

