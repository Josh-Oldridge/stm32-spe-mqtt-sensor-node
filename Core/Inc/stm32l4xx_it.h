/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l4xx_it.h
  * @brief   Interrupt Service Routine Prototypes for CN0575 Project
  * @details This header file declares interrupt handler prototypes for the STM32L496ZG-P
  *          Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project. It supports
  *          interrupt-driven operations for peripherals like SPI1 (ADIN1110 MAC-PHY),
  *          I2C2 (sensor communication), ADC1 (sensor data), and EXTI (ADIN1110 INT_N) to
  *          enable secure MQTT transmission of sensor data over TLSv1.2 via lwIP.
  * @addtogroup interrupts Interrupt Handlers
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
#ifndef __STM32L4xx_IT_H
#define __STM32L4xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void DMA1_Channel1_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);
void DMA1_Channel3_IRQHandler(void);
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void SPI1_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void TIM6_DAC_IRQHandler(void);
void DMA2_Channel6_IRQHandler(void);
void DMA2_Channel7_IRQHandler(void);
/* USER CODE BEGIN EFP */

/**
  * @fn void NMI_Handler(void)
  * @brief  Handles Non-Maskable Interrupt (NMI)
  * @details Processes critical system faults that cannot be masked, entering an infinite
  *          loop to halt execution in the CN0575 project.
  */

/**
  * @fn void HardFault_Handler(void)
  * @brief  Handles Hard Fault Interrupt
  * @details Processes severe hardware faults (e.g., invalid memory access) in the CN0575
  *          project, halting execution in an infinite loop.
  */

/**
  * @fn void MemManage_Handler(void)
  * @brief  Handles Memory Management Fault Interrupt
  * @details Manages memory protection faults in the CN0575 project, halting execution if triggered.
  */

/**
  * @fn void BusFault_Handler(void)
  * @brief  Handles Bus Fault Interrupt
  * @details Processes bus-related faults (e.g., prefetch or memory access errors) in the
  *          CN0575 project, halting execution if triggered.
  */

/**
  * @fn void UsageFault_Handler(void)
  * @brief  Handles Usage Fault Interrupt
  * @details Manages faults from undefined instructions or illegal states in the CN0575
  *          project, halting execution if triggered.
  */

/**
  * @fn void DebugMon_Handler(void)
  * @brief  Handles Debug Monitor Interrupt
  * @details Supports debug operations in the CN0575 project, typically used for debugging tools.
  */

/**
  * @fn void DMA1_Channel1_IRQHandler(void)
  * @brief  Handles DMA1 Channel 1 Interrupt (ADC1)
  * @details Manages DMA transfer completion for ADC1, used for sensor data sampling in the
  *          CN0575 project.
  */

/**
  * @fn void DMA1_Channel2_IRQHandler(void)
  * @brief  Handles DMA1 Channel 2 Interrupt (SPI1 RX)
  * @details Manages DMA receive completion for SPI1, handling data from the ADIN1110 in the
  *          CN0575 project.
  */

/**
  * @fn void DMA1_Channel3_IRQHandler(void)
  * @brief  Handles DMA1 Channel 3 Interrupt (SPI1 TX)
  * @details Manages DMA transmit completion for SPI1, sending data to the ADIN1110 in the
  *          CN0575 project.
  */

/**
  * @fn void DMA1_Channel4_IRQHandler(void)
  * @brief  Handles DMA1 Channel 4 Interrupt (I2C2 TX)
  * @details Manages DMA transmit completion for I2C2, used for sensor communication in the
  *          CN0575 project.
  */

/**
  * @fn void DMA1_Channel5_IRQHandler(void)
  * @brief  Handles DMA1 Channel 5 Interrupt (I2C2 RX)
  * @details Manages DMA receive completion for I2C2, handling sensor data in the CN0575 project.
  */

/**
  * @fn void I2C2_EV_IRQHandler(void)
  * @brief  Handles I2C2 Event Interrupt
  * @details Processes event-driven I2C2 interrupts (e.g., transfer complete) for sensor
  *          communication in the CN0575 project.
  */

/**
  * @fn void I2C2_ER_IRQHandler(void)
  * @brief  Handles I2C2 Error Interrupt
  * @details Manages I2C2 error conditions (e.g., bus errors) during sensor communication in
  *          the CN0575 project.
  */

/**
  * @fn void SPI1_IRQHandler(void)
  * @brief  Handles SPI1 Global Interrupt
  * @details Processes SPI1 interrupts (e.g., transfer complete) for ADIN1110 communication
  *          in the CN0575 project.
  */

/**
  * @fn void EXTI15_10_IRQHandler(void)
  * @brief  Handles EXTI Line [15:10] Interrupts
  * @details Manages external interrupts for ADIN1110 INT_N and reset button events in the
  *          CN0575 project.
  */

/**
  * @fn void TIM6_DAC_IRQHandler(void)
  * @brief  Handles TIM6 Global Interrupt (Timebase Source)
  * @details Processes TIM6 interrupts as the system timebase source, providing timing for
  *          FreeRTOS and other operations in the CN0575 project. Also handles DAC underrun
  *          errors if applicable.
  */

/**
  * @}
  */
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L4xx_IT_H */
