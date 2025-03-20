/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.h
  * @brief   SPI Interface Definitions for ADIN1110 in CN0575 Project
  * @details This header file declares functions and structures for SPI communication
  *          with the ADIN1110 MAC-PHY on the STM32L496ZG-P Nucleo board in the CN0575
  *          Single Pair Ethernet (SPE) board project. It supports secure MQTT transmission
  *          of sensor data over TLSv1.2 via lwIP and the ADIN1110. Includes initialization,
  *          chip select control, and SPI transaction functions.
  * @addtogroup spi SPI Module
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
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "boardsupport.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
/** @brief SPI handle for SPI1, used for ADIN1110 communication. */
extern SPI_HandleTypeDef hspi1;
/* USER CODE END Private defines */

void MX_SPI1_Init(void);

/* USER CODE BEGIN Prototypes */
/**
  * @fn void MX_SPI1_Init(void)
  * @brief  Initializes the SPI1 peripheral for ADIN1110
  * @details Configures SPI1 in master mode with parameters suited for the ADIN1110:
  *          - Full-duplex, 8-bit data, MSB first
  *          - CPOL high, CPHA 2-edge (Mode 3)
  *          - Software NSS, highest baud rate (prescaler 2)
  * @note    Called during system startup to prepare SPI1 for Ethernet operations.
  */

/**
  * @brief  Selects the ADIN1110 chip via CS pin
  * @details Sets the SPI1_CS pin low to enable SPI communication with the ADIN1110.
  */
void ADIN1110_CS_Select(void);

/**
  * @brief  Deselects the ADIN1110 chip via CS pin
  * @details Sets the SPI1_CS pin high to disable SPI communication with the ADIN1110.
  */
void ADIN1110_CS_Deselect(void);

/**
  * @brief  Performs an SPI write and read operation for ADIN1110
  * @param [in]  pBufferTx  Transmit buffer
  * @param [out] pBufferRx  Receive buffer
  * @param [in]  nbBytes    Number of bytes to transfer
  * @param [in]  useDma     True to use DMA, false for interrupt-based transfer
  * @return  HAL_StatusTypeDef indicating success or error
  * @details Handles chip select and initiates SPI transfer for ADIN1110 communication.
  */
HAL_StatusTypeDef   HAL_SPI_Write_Read      (uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes, bool useDma);

/**
  * @}
  */
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

