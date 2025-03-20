/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   GPIO Interface Definitions for CN0575 Project
  * @details This header file declares functions for configuring and managing GPIO pins
  *          on the STM32L496ZG-P Nucleo board in the CN0575 Single Pair Ethernet (SPE)
  *          board project. It supports interrupt handling for the ADIN1110 MAC-PHY (INT_N),
  *          LED control, and chip select operations for secure MQTT transmission of sensor
  *          data over TLSv1.2 via lwIP and the ADIN1110.
  * @addtogroup gpio GPIO Module
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "boardsupport.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
/**
  * @fn void MX_GPIO_Init(void)
  * @brief  Initializes GPIO pins for CN0575 project
  * @details Configures GPIO pins for inputs (e.g., interrupt, link status),
  *          outputs (e.g., LEDs, chip select, reset), and external interrupts
  *          (e.g., INT_N) on the STM32L496ZG-P for ADIN1110 operation.
  */

/**
  * @brief  Disables the external interrupt (INT_N)
  * @details Disables the IRQ associated with the ADIN1110's INT_N pin (EXTI15_10_IRQn)
  *          to stop interrupt handling.
  */
void HAL_INT_N_DisableIRQ(void);

/**
  * @brief  Enables the external interrupt (INT_N)
  * @details Enables and sets priority for the IRQ associated with the ADIN1110's INT_N pin
  *          (EXTI15_10_IRQn) to allow interrupt handling.
  */
void HAL_INT_N_EnableIRQ(void);

/**
  * @brief  Toggles the status LED
  * @details Toggles the GPIO pin (PB7) connected to the status LED on the STM32L496ZG-P
  *          board for visual indication.
  */
void MX_Led_Toggle(void);

/**
  * @brief  Gets the interrupt callback function
  * @return Pointer to the registered ADI_CB callback function for INT_N interrupts
  * @details Retrieves the callback set for handling ADIN1110 interrupt events.
  */
ADI_CB getIntCallback(void);

/**
  * @brief  Gets the interrupt callback parameter
  * @return Pointer to the user-defined parameter for the INT_N callback
  * @details Retrieves the parameter associated with the ADIN1110 interrupt callback.
  */
void *getIntCBParam(void);

/**
  * @}
  */
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

