/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   I2C Interface for CN0575 Project
  * @details This header declares functions and callbacks for I2C2 configuration on the
  *          STM32L496ZG-P Nucleo board in the CN0575 Single Pair Ethernet (SPE) board
  *          project. It supports communication with the ADXL345 accelerometer and TMP102
  *          temperature sensor via I2C2, used by freertos.c’s AccelTask and TempTask when
  *          USE_LWIP is defined. Includes semaphore synchronization for FreeRTOS tasks.
  * @addtogroup i2c I2C Configuration
  * @{
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
/** @brief Additional includes for FreeRTOS semaphore support */
#include "FreeRTOS.h"
#include "semphr.h"
/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN Private defines */

/** @fn extern I2C_HandleTypeDef hi2c1
  * @brief I2C1 handle for sensor communication
  */

/** @brief Semaphore for synchronizing I2C access in FreeRTOS tasks */
extern SemaphoreHandle_t i2cSemaphore;
/* USER CODE END Private defines */

void MX_I2C1_Init(void);

/* USER CODE BEGIN Prototypes */

/**
  * @brief Initialize I2C semaphore for FreeRTOS
  * @details Creates a binary semaphore for synchronizing I2C1 access, called from main.c.
  */
void I2C_InitSemaphore(void);

/**
  * @brief I2C1 master transmit complete callback
  * @param [in] hi2c I2C handle
  * @details Releases the I2C1 semaphore from ISR after transmit completes, used by sensor tasks.
  */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);

/**
  * @brief I2C1 master receive complete callback
  * @param [in] hi2c I2C handle
  * @details Releases the I2C1 semaphore from ISR after receive completes, used by sensor tasks.
  */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

/** @}*/
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

