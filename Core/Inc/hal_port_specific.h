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
  * @file    hal_port_specific.h
  * @brief   HAL Port-Specific Definitions for CN0575 Project
  * @details This header provides hardware abstraction layer (HAL) configurations for the
  *          STM32L496ZG-P Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project.
  *          Defines alignment macros, DMA thresholds, and interrupt settings for SPI1
  *          communication with the ADIN1110 MAC-PHY. Used in both standalone (non-USE_LWIP)
  *          and lwIP modes to optimize frame handling and interrupt behavior. Key active
  *          definitions include MIN_SIZE_FOR_DMA and ADI_EDGE_SENSITIVE_IRQ; others (e.g.,
  *          MDIO-related) are unused in this project.
  * @addtogroup hal HAL Configuration
  * @{
  ******************************************************************************
  */

#ifndef HAL_PORT_SPECIFIC_H
#define HAL_PORT_SPECIFIC_H

#include <stdlib.h>
#include <stdint.h>

#include "stm32l496xx.h"
#include "boardsupport.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRAGMA(x) _Pragma(#x)
#define ATTRIBUTE(x) __attribute__((x))

#if defined (__GNUC__)
  /* Gcc uses attributes */
  #define HAL_ALIGNED_PRAGMA(num)
  #define HAL_ALIGNED_ATTRIBUTE(num) ATTRIBUTE(aligned(num))
  #define HAL_UNUSED_ATTRIBUTE ATTRIBUTE(unused)
#elif defined ( __ICCARM__ )
  /* IAR uses a pragma */
  #define HAL_ALIGNED_ATTRIBUTE(num)
  #define HAL_ALIGNED_PRAGMA(num) PRAGMA(data_alignment=num)
  #define HAL_UNUSED_ATTRIBUTE
#elif defined (__CC_ARM)
  /* Keil uses a decorator which is placed in the same position as pragmas */
  #define HAL_ALIGNED_ATTRIBUTE(num)
  #define HAL_ALIGNED_PRAGMA(num) __align(##num)
  #define HAL_UNUSED_ATTRIBUTE ATTRIBUTE(unused)
#else
#error "Toolchain not supported"
#endif

#define DMA_BUFFER_ALIGN(var, alignBytes)   HAL_ALIGNED_PRAGMA(alignBytes) var HAL_ALIGNED_ATTRIBUTE(alignBytes)

/** @brief Minimum transaction size for enabling DMA in SPI transfers
  * @details Set to 16 bytes; SPI1 transactions (ADIN1110) use DMA if >= this size, otherwise
  *          interrupt-based, optimizing performance in both standalone and lwIP modes.
  */
#define MIN_SIZE_FOR_DMA            (16)

/** @brief Duration of an MDIO read in microseconds (unused in CN0575)
  * @details Set to 1250 µs; intended for MDIO timeout conversion but unused as ADIN1110 uses SPI1.
  */
#define ADI_HAL_MDIO_READ_DURATION  (1250)

/** @brief Option to pause RX_RDY interrupt if no buffers are available (unused in CN0575)
  * @details Set to 0; intended for Generic SPI protocol but irrelevant as ADIN1110 uses OPEN
  *          Alliance SPI with NORX bit handling, not active in this project.
  */
#define ADI_PAUSE_RX_IF_NO_BUFFERS  (0)

/** @brief Indicates if the host IRQ is edge or level sensitive
  * @details Set to 1 (edge-sensitive) for EXTI15_10_IRQn (ADIN1110 INT_N on PF12), matching
  *          GPIO_MODE_IT_FALLING in gpio.c for interrupt handling in the CN0575 project.
  */
#define ADI_EDGE_SENSITIVE_IRQ      (1)

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

#endif /* HAL_PORT_SPECIFIC_H */
