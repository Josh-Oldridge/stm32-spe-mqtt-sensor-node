/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   GPIO Configuration for CN0575 Project
 * @details This file provides code for configuring GPIO pins on the STM32L496ZG-P
 *          Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project. It
 *          initializes pins for inputs (e.g., interrupt, link status), outputs (e.g.,
 *          LEDs, chip select, reset), and external interrupts (e.g., INT_N) for the
 *          ADIN1110 MAC-PHY. Supports secure MQTT transmission over TLSv1.2 via lwIP.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
/** @brief Interrupt callback function pointer for INT_N, set via HAL_INT_N_Register_Callback. */
static          ADI_CB gpfIntCallback = NULL;

/** @brief User-defined parameter for INT_N callback, set via HAL_INT_N_Register_Callback. */
static void     *gpIntCBParam = NULL;
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
     PF0   ------> I2C2_SDA
     PF1   ------> I2C2_SCL
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CN0575_ALERT_LED_GPIO_Port, CN0575_ALERT_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : Reset_Button_Pin */
  GPIO_InitStruct.Pin = Reset_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(Reset_Button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PF0 PF1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : Interrupt_Pin */
  GPIO_InitStruct.Pin = Interrupt_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Interrupt_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Link_Status_Pin */
  GPIO_InitStruct.Pin = Link_Status_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Link_Status_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CN0575_ALERT_LED_Pin */
  GPIO_InitStruct.Pin = CN0575_ALERT_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CN0575_ALERT_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_CS_Pin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Reset_Pin */
  GPIO_InitStruct.Pin = Reset_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Reset_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD1_Pin */
  GPIO_InitStruct.Pin = LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD1_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/**
  * @brief  Gets the interrupt callback function
  * @return Pointer to the registered ADI_CB callback function for INT_N interrupts
  * @details Retrieves the callback set for handling ADIN1110 interrupt events.
  */
ADI_CB getIntCallback(void) {
    return gpfIntCallback;
}

/**
  * @brief  Gets the interrupt callback parameter
  * @return Pointer to the user-defined parameter for the INT_N callback
  * @details Retrieves the parameter associated with the ADIN1110 interrupt callback.
  */
void *getIntCBParam(void) {
    return gpIntCBParam;
}

/**
  * @brief  Registers an interrupt callback for INT_N
  * @param [in] pfCallback  Callback function to register
  * @param [in] pCBParam    User-defined parameter for callback
  * @return  0 on success
  * @details Sets the global interrupt callback and parameter for handling ADIN1110
  *          INT_N interrupt events, with priority set to 0.
  */
uint32_t HAL_INT_N_Register_Callback(ADI_CB const *pfCallback,
		void *const pCBParam) {
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);

	gpfIntCallback = (ADI_CB) pfCallback;
	gpIntCBParam = pCBParam;

	return 0;
}

/**
  * @brief  Disables the external interrupt (INT_N)
  * @details Disables the IRQ associated with the ADIN1110's INT_N pin (EXTI15_10_IRQn)
  *          to stop interrupt handling.
  */
void HAL_INT_N_DisableIRQ(void) {
	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief  Enables the external interrupt (INT_N)
  * @details Enables and sets priority for the IRQ associated with the ADIN1110's INT_N pin
  *          (EXTI15_10_IRQn) to allow interrupt handling.
  */
void HAL_INT_N_EnableIRQ(void) {
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief  Toggles the status LED
  * @details Toggles the GPIO pin (PB7) connected to the status LED on the STM32L496ZG-P
  *          board for visual indication.
  */
void MX_Led_Toggle(void) {
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
}

/**
  * @}
  */
/* USER CODE END 2 */
