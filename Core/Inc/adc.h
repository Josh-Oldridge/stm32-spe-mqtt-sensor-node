/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
  * @details This file defines the interface for configuring the ADC1 peripheral
  *          on the STM32L496ZG-P Nucleo board. The ADC is used to sample analog
  *          sensor data from the ACS723 Current Sensor module, which is then transmitted
  *          over MQTT using lwIP and mbedtls for secure communication.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
/** @addtogroup adc ADC Module
 *  @{
 */

/**
 * @brief Size of the ADC buffer for storing conversion results.
 */
#define ADC_BUFFER_SIZE 10
/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */
/**
 * @brief Buffer to store ADC conversion results.
 * @details This buffer holds the raw 12-bit ADC values sampled from the sensor
 *          connected to ADC1 channel 8 (PA3).
 */
extern uint16_t adcBuffer[ADC_BUFFER_SIZE];
/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */
/**
 * @fn     void MX_ADC1_Init(void)
 * @brief  Initializes the ADC1 peripheral.
 * @details Configures ADC1 for continuous conversion with DMA to sample sensor
 *          data from the CN0575 board. See adc.c for implementation details.
 * @retval None
 */

/** @} */ /* End of group adc */
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

