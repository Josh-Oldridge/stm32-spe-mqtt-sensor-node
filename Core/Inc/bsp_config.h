/*
 *---------------------------------------------------------------------------
 *
 * Copyright (c) 2020, 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.
 * and its licensors.By using this software you agree to the terms of the
 * associated Analog Devices Software License Agreement.
 *
 *---------------------------------------------------------------------------
 */

/**
  ******************************************************************************
  * @file    bsp_config.h
  * @brief   BSP Configuration for STM32L496ZG-P Nucleo Board in CN0575 Project
  * @details Defines hardware pin mappings and clock configurations for the STM32L496ZG-P
  *          Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project. Configures
  *          SPI1 for ADIN1110 MAC-PHY communication, GPIO pins for LEDs and interrupts,
  *          and system clock settings (80 MHz SYSCLK). Only a subset of definitions (e.g.,
  *          SPI1, ETH_INT_N, LED pins) is used in main.c and boardsupport.c; others (e.g.,
  *          MDIO pins) are unused legacy options.
  * @addtogroup bsp Board Support Package
  * @{
  ******************************************************************************
  */

#ifndef BSP_CONFIG_H
#define BSP_CONFIG_H



#if defined(EVAL_ADIN1110EBZ)
#define ETH_RESET_Pin GPIO_PIN_7
#define ETH_RESET_GPIO_Port GPIOC
#else
/*
 * Note: ETH_RESET_Pin is defined as GPIO_PIN_15 on GPIOD for non-EVAL_ADIN1110EBZ boards,
 *       matching Reset_Pin in main.h, used in BSP_HWReset.
 */
#define ETH_RESET_Pin GPIO_PIN_15
#define ETH_RESET_GPIO_Port GPIOD
#endif

/*
 * Note: ETH_SPI is defined as SPI1, matching main.c usage, despite BSP naming suggesting SPI2.
 */
#define ETH_SPI                 SPI1
#define ETH_SPI_IRQn            SPI1_IRQn
#define ETH_SPI_IRQ_HANDLER     SPI1_IRQHandler
#define ETH_SPI_CLK_ENABLE		__HAL_RCC_SPI1_CLK_ENABLE
#define ETH_SPI_CLK_DISABLE 	__HAL_RCC_SPI1_CLK_DISABLE

#define ETH_SPI_SS_Pin          GPIO_PIN_14
#define ETH_SPI_SS_GPIO_Port    GPIOD
#define ETH_SPI_MOSI_Pin        GPIO_PIN_7
#define ETH_SPI_MOSI_GPIO_Port  GPIOA
#define ETH_SPI_MOSI_AF         GPIO_AF5_SPI1
#if defined(USE_NUCLEO)
#define ETH_SPI_MISO_Pin        GPIO_PIN_6
#define ETH_SPI_MISO_GPIO_Port  GPIOA
#else
#define ETH_SPI_MISO_Pin        GPIO_PIN_14
#define ETH_SPI_MISO_GPIO_Port  GPIOB
#endif
#define ETH_SPI_MISO_AF         GPIO_AF5_SPI1
#define ETH_SPI_CLK_Pin         GPIO_PIN_5
#define ETH_SPI_CLK_GPIO_Port   GPIOA
#define ETH_SPI_CLK_AF          GPIO_AF5_SPI1


#define ETH_SPI_DMA_REQ_TX      DMA_REQUEST_3
#define ETH_SPI_DMA_REQ_RX      DMA_REQUEST_2

#if defined(EVAL_ADIN1110EBZ)
#define ETH_INT_N_Pin           GPIO_PIN_11
#define ETH_INT_N_GPIO_Port     GPIOB
#else
/*
 * Note: ETH_INT_N_Pin is defined as GPIO_PIN_12 on GPIOF for non-EVAL_ADIN1110EBZ boards,
 *       matching Interrupt_Pin in main.h, used for ADIN1110 INT_N interrupts.
 */
#define ETH_INT_N_Pin           GPIO_PIN_12
#define ETH_INT_N_GPIO_Port     GPIOF
#endif
#define ETH_INT_N_IRQn          EXTI15_10_IRQn
#define ETH_INT_N_IRQ_HANDLER   EXTI15_10_IRQHandler

#if defined(EVAL_ADIN1110EBZ)
#define BSP_LED1_PORT           GPIOC
#define BSP_LED1_PIN            GPIO_PIN_13
#define BSP_LED2_PORT           GPIOE
#define BSP_LED2_PIN            GPIO_PIN_2
#define BSP_LED3_PORT           GPIOE
#define BSP_LED3_PIN            GPIO_PIN_6
#define BSP_LED4_PORT           GPIOG
#define BSP_LED4_PIN            GPIO_PIN_15
#else
/*
 * Note: BSP_LED1_PORT (GPIOC, PIN_7), BSP_LED2_PORT (GPIOB, PIN_7), BSP_LED3_PORT (GPIOB, PIN_14)
 *       match LD1, LD2, LD3 in main.h, used for interrupt status, error, and heartbeat indicators.
 */
#define BSP_LED1_PORT           GPIOC
#define BSP_LED1_PIN            GPIO_PIN_7
#define BSP_LED2_PORT           GPIOB
#define BSP_LED2_PIN            GPIO_PIN_7
#define BSP_LED3_PORT           GPIOB
#define BSP_LED3_PIN            GPIO_PIN_14
//#define BSP_LED4_PORT           GPIOB
//#define BSP_LED4_PIN            GPIO_PIN_2
//#define BSP_LED5_PORT           GPIOB
//#define BSP_LED5_PIN            GPIO_PIN_10
#endif

#define ETH_GPIO_ENABLE         do { \
                                    __HAL_RCC_GPIOA_CLK_ENABLE(); \
                                    __HAL_RCC_GPIOB_CLK_ENABLE(); \
                                    __HAL_RCC_GPIOC_CLK_ENABLE(); \
                                    __HAL_RCC_GPIOD_CLK_ENABLE(); \
                                    __HAL_RCC_GPIOE_CLK_ENABLE(); \
                                    __HAL_RCC_GPIOF_CLK_ENABLE(); \
                                    __HAL_RCC_GPIOG_CLK_ENABLE(); \
                                    __HAL_RCC_GPIOH_CLK_ENABLE(); \
                                } while(0)

/* Assign one of the symbols defined below to configure SYSCLK/SCLK frequencies */
/*
 * Note: BSP_CLK_CFG set to BSP_CLK_CFG_96_24 by default, but main.c uses 80 MHz (PLLN=10, PLLR=DIV2),
 *       indicating this config is overridden in main.c’s SystemClock_Config.
 */
#define BSP_CLK_CFG             (BSP_CLK_CFG_96_24)

#define BSP_CLK_CFG_96_24       (0)
#define BSP_CLK_CFG_25_12P5     (1)
#define BSP_CLK_CFG_120_15      (2)
#define BSP_CLK_CFG_100_25      (3)
#define BSP_CLK_CFG_80_5        (4)
#define BSP_CLK_CFG_100_12P5    (5)

#if (BSP_CLK_CFG == BSP_CLK_CFG_96_24)

#define BSP_CLK_CFG_PLLN                (48)
#define BSP_CLK_CFG_PLLR                (RCC_PLLR_DIV2)
#define BSP_CLK_CFG_APB1CLKDIV          (RCC_HCLK_DIV2)
#define BSP_CLK_CFG_SPI_BAUDPRESCALER   (SPI_BAUDRATEPRESCALER_2)
#define BSP_CLK_CFG_FLASH_LATENCY       (FLASH_LATENCY_4)

#elif (BSP_CLK_CFG == BSP_CLK_CFG_25_12P5)

#define BSP_CLK_CFG_PLLN                (25)
#define BSP_CLK_CFG_PLLR                (RCC_PLLR_DIV4)
#define BSP_CLK_CFG_APB1CLKDIV          (RCC_HCLK_DIV1)
#define BSP_CLK_CFG_SPI_BAUDPRESCALER   (SPI_BAUDRATEPRESCALER_2)
#define BSP_CLK_CFG_FLASH_LATENCY       (FLASH_LATENCY_1)

#elif (BSP_CLK_CFG == BSP_CLK_CFG_120_15)

#define BSP_CLK_CFG_PLLN                (60)
#define BSP_CLK_CFG_PLLR                (RCC_PLLR_DIV2)
#define BSP_CLK_CFG_APB1CLKDIV          (RCC_HCLK_DIV4)
#define BSP_CLK_CFG_SPI_BAUDPRESCALER   (SPI_BAUDRATEPRESCALER_2)
#define BSP_CLK_CFG_FLASH_LATENCY       (FLASH_LATENCY_5)

#elif (BSP_CLK_CFG == BSP_CLK_CFG_100_25)

#define BSP_CLK_CFG_PLLN                (50)
#define BSP_CLK_CFG_PLLR                (RCC_PLLR_DIV2)
#define BSP_CLK_CFG_APB1CLKDIV          (RCC_HCLK_DIV2)
#define BSP_CLK_CFG_SPI_BAUDPRESCALER   (SPI_BAUDRATEPRESCALER_2)
#define BSP_CLK_CFG_FLASH_LATENCY       (FLASH_LATENCY_4)

#elif (BSP_CLK_CFG == BSP_CLK_CFG_80_5)

#define BSP_CLK_CFG_PLLN                (40)
#define BSP_CLK_CFG_PLLR                (RCC_PLLR_DIV2)
#define BSP_CLK_CFG_APB1CLKDIV          (RCC_HCLK_DIV1)
#define BSP_CLK_CFG_SPI_BAUDPRESCALER   (SPI_BAUDRATEPRESCALER_16)
#define BSP_CLK_CFG_FLASH_LATENCY       (FLASH_LATENCY_3)

#elif (BSP_CLK_CFG == BSP_CLK_CFG_100_12P5)

#define BSP_CLK_CFG_PLLN                (50)
#define BSP_CLK_CFG_PLLR                (RCC_PLLR_DIV2)
#define BSP_CLK_CFG_APB1CLKDIV          (RCC_HCLK_DIV2)
#define BSP_CLK_CFG_SPI_BAUDPRESCALER   (SPI_BAUDRATEPRESCALER_4)
#define BSP_CLK_CFG_FLASH_LATENCY       (FLASH_LATENCY_4)

#else

// FIXME: assert error

#endif

#endif /* BSP_CONFIG_H */
