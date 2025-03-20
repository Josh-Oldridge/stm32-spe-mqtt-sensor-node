/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.c
  * @brief   SPI Configuration and Communication for ADIN1110 in CN0575 Project
  * @details This file provides initialization and communication functions for the SPI1
  *          peripheral on the STM32L496ZG-P Nucleo board, used to interface with the
  *          ADIN1110 MAC-PHY in the CN0575 Single Pair Ethernet (SPE) board project.
  *          It supports secure MQTT transmission of sensor data over TLSv1.2 via lwIP
  *          and the ADIN1110. Includes SPI configuration, chip select control, DMA-based
  *          transfers, and callback handling for non-blocking operations.
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

/* Includes ------------------------------------------------------------------*/
#include "spi.h"

/* USER CODE BEGIN 0 */
#include "boardsupport.h"

/** @brief SPI callback function pointer, set via HAL_SPI_Register_Callback. */
static          ADI_CB gpfSpiCallback = NULL;

/** @brief User-defined parameter for SPI callback, set via HAL_SPI_Register_Callback. */
static void     *gpSpiCBParam = NULL;
/* USER CODE END 0 */

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

/* SPI1 init function */
void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */
  /**
	* @fn void MX_SPI1_Init(void)
	* @brief  Initializes the SPI1 peripheral for ADIN1110 communication
	* @details Configures SPI1 in master mode with settings optimized for the ADIN1110:
	*          - Full-duplex, 8-bit data, MSB first
	*          - CPOL high, CPHA 2-edge (Mode 3)
	*          - Software NSS, no CRC, highest baud rate (prescaler 2)
	*          - TI mode and NSS pulse disabled
	* @note    This function is called during system initialization to prepare SPI1 for
	*          Ethernet frame transfers with the ADIN1110 in the CN0575 project.
	*/
  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */
  /**
	* @fn void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
	* @brief  Initializes MSP for SPI1
	* @details Configures GPIO pins and DMA channels for SPI1:
	*          - PA5: SCK, PA6: MISO, PA7: MOSI (AF5)
	*          - DMA1 Channel 2: SPI1_RX, Channel 3: SPI1_TX
	*          - DMA settings: Memory increment, very high priority, byte alignment
	* @param [in] spiHandle  Pointer to SPI handle structure
	* @note    This function is called by HAL_SPI_Init() to set up hardware resources.
	*/
  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* SPI1 DMA Init */
    /* SPI1_RX Init */
    hdma_spi1_rx.Instance = DMA1_Channel2;
    hdma_spi1_rx.Init.Request = DMA_REQUEST_1;
    hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_rx.Init.Mode = DMA_NORMAL;
    hdma_spi1_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_spi1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(spiHandle,hdmarx,hdma_spi1_rx);

    /* SPI1_TX Init */
    hdma_spi1_tx.Instance = DMA1_Channel3;
    hdma_spi1_tx.Init.Request = DMA_REQUEST_1;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(spiHandle,hdmatx,hdma_spi1_tx);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */
  /**
	* @fn void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
	 * @brief  Deinitializes MSP for SPI1
	 * @details Resets GPIO pins and DMA channels for SPI1, disabling the peripheral clock.
	 * @param [in] spiHandle  Pointer to SPI handle structure
	 * @note    This function is called by HAL_SPI_DeInit() to release hardware resources.
	 */
  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

    /* SPI1 DMA DeInit */
    HAL_DMA_DeInit(spiHandle->hdmarx);
    HAL_DMA_DeInit(spiHandle->hdmatx);

    /* SPI1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SPI1_IRQn);
  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/**
  * @brief  Selects the ADIN1110 chip via CS pin
  * @details Sets the SPI1_CS pin low to enable communication with the ADIN1110.
  * @note    Called before SPI transactions to select the device.
  */
void ADIN1110_CS_Select(void)
{
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
}

/**
  * @brief  Deselects the ADIN1110 chip via CS pin
  * @details Sets the SPI1_CS pin high to disable communication with the ADIN1110.
  * @note    Called after SPI transactions to release the device.
  */
void ADIN1110_CS_Deselect(void)
{
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief  Performs an SPI write and read operation for ADIN1110
  * @param [in]  pBufferTx  Transmit buffer
  * @param [out] pBufferRx  Receive buffer
  * @param [in]  nbBytes    Number of bytes to transfer
  * @param [in]  useDma     True to use DMA, false for interrupt-based transfer
  * @return  HAL_StatusTypeDef indicating success or error
  * @details Selects the ADIN1110, initiates an SPI transfer using DMA or interrupts,
  *          and handles chip select. Used for non-blocking SPI communication in the
  *          CN0575 project’s Ethernet frame handling.
  */
HAL_StatusTypeDef HAL_SPI_Write_Read(uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes, bool useDma)
{
    HAL_StatusTypeDef   status;

    ADIN1110_CS_Select();

    if (useDma)
    {
        status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)pBufferTx, (uint8_t *)pBufferRx, nbBytes);
    }
    else
    {
        status = HAL_SPI_TransmitReceive_IT(&hEthSpi, (uint8_t *)pBufferTx, (uint8_t *)pBufferRx, nbBytes);
    }

    return status;
}

/**
  * @brief  Registers an SPI callback function
  * @param [in] pfCallback  Callback function to register
  * @param [in] pCBParam    User-defined parameter for callback
  * @return  0 on success
  * @details Sets the global SPI callback and parameter for handling SPI transaction
  *          completion events in the CN0575 project.
  */
uint32_t HAL_SPI_Register_Callback(ADI_CB const *pfCallback, void *const pCBParam)
{
    gpfSpiCallback = (ADI_CB)pfCallback;
    gpSpiCBParam = pCBParam ;

    return 0;
}

/**
  * @brief  SPI transmit/receive complete callback
  * @param [in] hspi  Pointer to SPI handle
  * @details Deselects the ADIN1110 and invokes the registered callback if set.
  *          Ensures the CS pin is released after each transaction.
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
 {

 	if (hspi->Instance == SPI1) {
		ADIN1110_CS_Deselect();
		if (gpfSpiCallback != NULL) {
		    (*gpfSpiCallback)(gpSpiCBParam, 0, NULL);
		}
	}
}

/**
 * @}
 */
/* USER CODE END 1 */
