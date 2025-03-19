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
 * @file    hal.h
 * @brief   Hardware Abstraction Layer (HAL) Interface for STM32L496ZG-P in CN0575 Project
 * @details This file defines the Hardware Abstraction Layer (HAL) interface for the STM32L496ZG-P
 *          Nucleo board, used in the CN0575 Single Pair Ethernet (SPE) board project by Analog
 *          Devices. It provides abstractions for low-level hardware operations, including
 *          interrupt management, SPI communication with the ADIN1110 MAC-PHY, and optional
 *          MDIO access for PHY operations (ADIN1100). The HAL supports integration with lwIP
 *          and ADIN1110 drivers for secure MQTT transmission of sensor data over TLSv1.2.
 *          Conditional compilation allows flexibility for different hardware configurations
 *          and device handle usage. This header is intended for use by developers integrating
 *          the CN0575 board into embedded systems requiring Ethernet connectivity.
 */

/** @addtogroup hal HAL API
 *  @{
 */

#ifndef HAL_H
#define HAL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "adi_eth_common.h"
#include "hal_port_specific.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief   HAL return code indicating successful execution
 * @details Defines a constant return value of 0 to indicate that a HAL function executed
 *          without errors. Used across all HAL API functions for consistency.
 */
#define ADI_HAL_SUCCESS         (0)

/**
 * @brief   HAL return code indicating an error
 * @details Defines a constant return value of 1 to indicate that a HAL function encountered
 *          an error during execution. Specific error details depend on the function and
 *          platform implementation.
 */
#define ADI_HAL_ERROR           (1)

/**
 * @brief   Disables all processor interrupts
 * @details Uses the CMSIS intrinsic `__disable_irq()` to disable all interrupts on the
 *          STM32L496ZG-P, ensuring atomic operations in critical sections. Typically paired
 *          with ADI_HAL_EXIT_CRITICAL_SECTION for thread safety.
 */
#define     ADI_HAL_ENTER_CRITICAL_SECTION(...)     __disable_irq()

/**
 * @brief   Re-enables processor interrupts
 * @details Uses the CMSIS intrinsic `__enable_irq()` to restore interrupt functionality on
 *          the STM32L496ZG-P after a critical section. Must follow a corresponding
 *          ADI_HAL_ENTER_CRITICAL_SECTION call.
 */
#define     ADI_HAL_EXIT_CRITICAL_SECTION(...)      __enable_irq()

/**
 * @brief   Callback function type for HAL asynchronous events
 * @param [in] pCBParam  Client-supplied parameter passed to the callback for context
 * @param [in] Event     Event ID specific to the driver or service (e.g., interrupt or SPI completion)
 * @param [in] pArg      Pointer to event-specific data or arguments
 * @details Defines the signature for callback functions used in the HAL for handling
 *          asynchronous events, such as ADIN1110 interrupts or SPI transaction completions
 *          in the CN0575 project. The callback is registered via HAL_RegisterCallback or
 *          HAL_SpiRegisterCallback.
 */
typedef void (*HAL_Callback_t)(void *pCBParam, /*!< Client-supplied callback parameter. */
uint32_t Event, /*!< Event ID specific to the Driver/Service. */
void *pArg /*!< Pointer to the event-specific argument. */
);

#if !defined(ADI_HAL_USE_DEVICE_HANDLE)

/**
 * @name HAL Wrappers (No Device Handle)
 * @brief These macros strip the device handle from arguments before invoking HAL functions,
 *        providing a simplified interface when device handles are not required. If the
 *        platform needs device handles, define ADI_HAL_USE_DEVICE_HANDLE and provide
 *        platform-specific prototypes in hal_port_specific.h.
 */

/** @{ */

/**
 * @brief   Initializes the HAL without a device handle
 * @param [in] dev  Device driver handle (ignored in this mode)
 * @param [in] ...  Additional arguments passed to HAL_Init_Hook()
 * @details Wrapper macro that calls HAL_Init_Hook() without passing the device handle,
 *          used during driver initialization for platform-specific setup in the CN0575 project.
 */
#define     ADI_HAL_INIT(dev, ...)                  HAL_Init_Hook(__VA_ARGS__)

/**
 * @brief   Uninitializes the HAL without a device handle
 * @param [in] dev  Device driver handle (ignored in this mode)
 * @param [in] ...  Additional arguments passed to HAL_UnInit_Hook()
 * @details Wrapper macro that calls HAL_UnInit_Hook() without passing the device handle,
 *          used during driver cleanup in the CN0575 project.
 */
#define     ADI_HAL_UNINIT(dev, ...)                HAL_UnInit_Hook(__VA_ARGS__)

#if defined(ADIN1100)
/**
 * @brief   Reads from a PHY register without a device handle
 * @param [in] dev  Device driver handle (ignored in this mode)
 * @param [in] ...  Arguments: phyAddr, regAddr, data (passed to HAL_PhyRead)
 * @details Wrapper macro that calls HAL_PhyRead() for MDIO-based PHY register reads,
 *          stripping the device handle. Specific to ADIN1100 in the CN0575 project.
 */
#define     ADI_HAL_PHY_READ(dev, ...)              HAL_PhyRead(__VA_ARGS__)

/**
 * @brief   Writes to a PHY register without a device handle
 * @param [in] dev  Device driver handle (ignored in this mode)
 * @param [in] ...  Arguments: phyAddr, regAddr, data (passed to HAL_PhyWrite)
 * @details Wrapper macro that calls HAL_PhyWrite() for MDIO-based PHY register writes,
 *          stripping the device handle. Specific to ADIN1100 in the CN0575 project.
 */     ADI_HAL_PHY_WRITE(dev, ...)             HAL_PhyWrite(__VA_ARGS__)

#endif

/**
 * @brief   Enables external interrupts without a device handle
 * @param [in] dev  Device driver handle (ignored in this mode)
 * @param [in] ...  Additional arguments (if any, passed to HAL_EnableIrq)
 * @details Wrapper macro that calls HAL_EnableIrq() to enable the ADIN1110 interrupt line
 *          (INT_N) on the STM32L496ZG-P, stripping the device handle.
 */
#define     ADI_HAL_ENABLE_IRQ(dev, ...)            HAL_EnableIrq(__VA_ARGS__)

/**
 * @brief   Disables external interrupts without a device handle
 * @param [in] dev  Device driver handle (ignored in this mode)
 * @param [in] ...  Additional arguments (if any, passed to HAL_DisableIrq)
 * @details Wrapper macro that calls HAL_DisableIrq() to disable the ADIN1110 interrupt line
 *          (INT_N) on the STM32L496ZG-P, stripping the device handle.
 */
#define     ADI_HAL_DISABLE_IRQ(dev, ...)           HAL_DisableIrq(__VA_ARGS__)

/**
 * @brief   Gets interrupt enable status without a device handle
 * @param [in] dev  Device driver handle (ignored in this mode)
 * @param [in] ...  Additional arguments (if any, passed to HAL_GetEnableIrq)
 * @details Wrapper macro that calls HAL_GetEnableIrq() to check the ADIN1110 interrupt
 *          status, stripping the device handle.
 */
#define     ADI_HAL_GET_ENABLE_IRQ(dev, ...)        HAL_GetEnableIrq(__VA_ARGS__)

/**
 * @brief   Sets an interrupt as pending without a device handle
 * @param [in] dev  Device driver handle (ignored in this mode)
 * @param [in] ...  Additional arguments (if any, passed to HAL_SetPendingIrq)
 * @details Wrapper macro that calls HAL_SetPendingIrq() to mark the ADIN1110 interrupt as
 *          pending, stripping the device handle.
 */
#define     ADI_HAL_SET_PENDING_IRQ(dev, ...)       HAL_SetPendingIrq(__VA_ARGS__)

/**
 * @brief   Gets pending interrupt status without a device handle
 * @param [in] dev  Device driver handle (ignored in this mode)
 * @param [in] ...  Additional arguments (if any, passed to HAL_GetPendingIrq)
 * @details Wrapper macro that calls HAL_GetPendingIrq() to check if the ADIN1110 interrupt
 *          is pending, stripping the device handle.
 */
#define     ADI_HAL_GET_PENDING_IRQ(dev, ...)       HAL_GetPendingIrq(__VA_ARGS__)

/**
 * @brief   Registers an interrupt callback without a device handle
 * @param [in] dev  Device driver handle (ignored in this mode)
 * @param [in] ...  Arguments: intCallback, hDevice (passed to HAL_RegisterCallback)
 * @details Wrapper macro that calls HAL_RegisterCallback() to set an interrupt callback
 *          for the ADIN1110, stripping the device handle.
 */
#define     ADI_HAL_REGISTER_CALLBACK(dev, ...)     HAL_RegisterCallback(__VA_ARGS__)

#if defined(ADIN1110) || defined(ADIN2111)
 /**
  * @brief   Registers an SPI callback without a device handle
  * @param [in] dev  Device driver handle (ignored in this mode)
  * @param [in] ...  Arguments: spiCallback, hDevice (passed to HAL_SpiRegisterCallback)
  * @details Wrapper macro that calls HAL_SpiRegisterCallback() to set an SPI callback for
  *          ADIN1110/2111, stripping the device handle.
  */
#define     ADI_HAL_SPI_REGISTER_CALLBACK(dev, ...) HAL_SpiRegisterCallback(__VA_ARGS__)

 /**
  * @brief   Performs SPI read/write without a device handle
  * @param [in] dev  Device driver handle (ignored in this mode)
  * @param [in] ...  Arguments: pBufferTx, pBufferRx, nbBytes, useDma (passed to HAL_SpiReadWrite)
  * @details Wrapper macro that calls HAL_SpiReadWrite() for SPI transactions with the
  *          ADIN1110/2111, stripping the device handle.
  */
#define     ADI_HAL_SPI_READ_WRITE(dev, ...)        HAL_SpiReadWrite(__VA_ARGS__)

 /**
  * @brief   Initializes the FCS calculator without a device handle
  * @param [in] dev  Device driver handle (ignored in this mode)
  * @param [in] ...  Additional arguments (if any, passed to HAL_FcsInit)
  * @details Wrapper macro that calls HAL_FcsInit() to prepare the FCS calculator for
  *          Ethernet frame checks, stripping the device handle.
  */
#define     ADI_HAL_FCS_INIT(dev, ...)              HAL_FcsInit(__VA_ARGS__)

 /**
  * @brief   Uninitializes the FCS calculator without a device handle
  * @param [in] dev  Device driver handle (ignored in this mode)
  * @param [in] ...  Additional arguments (if any, passed to HAL_FcsUnInit)
  * @details Wrapper macro that calls HAL_FcsUnInit() to clean up the FCS calculator,
  *          stripping the device handle.
  */
#define     ADI_HAL_FCS_UNINIT(dev, ...)            HAL_FcsUnInit(__VA_ARGS__)

 /**
  * @brief   Calculates FCS without a device handle
  * @param [in] dev  Device driver handle (ignored in this mode)
  * @param [in] ...  Arguments: pBuf, nbBytes (passed to HAL_FcsCalculate)
  * @details Wrapper macro that calls HAL_FcsCalculate() to compute the FCS for Ethernet
  *          frames, stripping the device handle.
  */
#define     ADI_HAL_FCS_CALCULATE(dev, ...)         HAL_FcsCalculate(__VA_ARGS__)

#endif

/** @} */

 /**
  * @name Standard HAL Functions
  * @brief These functions provide the core HAL implementation when ADI_HAL_USE_DEVICE_HANDLE
  *        is not defined, meaning they do not receive a device handle. For device handle
  *        support, define ADI_HAL_USE_DEVICE_HANDLE and implement prototypes in
  *        hal_port_specific.h.
  */

/** @{ */

#if defined(ADIN1100)

/**
 * @brief   Reads from a PHY register via MDIO
 * @param [in]  phyAddr  PHY address on the MDIO bus
 * @param [in]  regAddr  Register address (DEVTYPE + ADDR format)
 * @param [out] data     Pointer to store the 16-bit register value
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Implements a blocking read from a PHY register using the MDIO interface,
 *          specific to the ADIN1100 in the CN0575 project. Used for PHY configuration
 *          and status checking.
 * @sa      HAL_PhyWrite()
 */
uint32_t        HAL_PhyRead             (uint8_t phyAddr, uint32_t regAddr, uint16_t *data);

 /**
  * @brief   Writes to a PHY register via MDIO
  * @param [in] phyAddr  PHY address on the MDIO bus
  * @param [in] regAddr  Register address (DEVTYPE + ADDR format)
  * @param [in] data     16-bit value to write
  * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
  * @details Implements a blocking write to a PHY register using the MDIO interface,
  *          specific to the ADIN1100 in the CN0575 project. Used for PHY configuration.
  * @sa      HAL_PhyRead()
  */
uint32_t        HAL_PhyWrite            (uint8_t phyAddr, uint32_t regAddr, uint16_t data);
#endif

/**
 * @brief   Enables the external interrupt (INT_N)
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Enables the interrupt line (INT_N) for the ADIN1110 on the STM32L496ZG-P,
 *          allowing the processor to respond to Ethernet events in the CN0575 project.
 * @sa      HAL_DisableIrq()
 */
uint32_t HAL_EnableIrq(void);

/**
 * @brief   Disables the external interrupt (INT_N)
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Disables the interrupt line (INT_N) for the ADIN1110 on the STM32L496ZG-P,
 *          preventing interrupt triggers in the CN0575 project.
 * @sa      HAL_EnableIrq()
 */
uint32_t HAL_DisableIrq(void);

/**
 * @brief   Gets the enable status of the external interrupt (INT_N)
 * @return  1 if enabled, 0 if disabled
 * @details Checks whether the ADIN1110 interrupt is enabled on the STM32L496ZG-P in
 *          the CN0575 project, useful for debugging or state management.
 */
uint32_t HAL_GetEnableIrq(void);

/**
 * @brief   Sets the external interrupt (INT_N) as pending
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Marks the ADIN1110 interrupt as pending on the STM32L496ZG-P, triggering the
 *          IRQ handler for deferred operations in the CN0575 project.
 */
uint32_t HAL_SetPendingIrq(void);

/**
 * @brief   Gets the pending status of the external interrupt (INT_N)
 * @return  1 if pending, 0 if not pending
 * @details Checks if the ADIN1110 interrupt is pending on the STM32L496ZG-P in the
 *          CN0575 project, aiding in interrupt handling logic.
 */
uint32_t HAL_GetPendingIrq(void);

/**
 * @brief   Registers a callback for device interrupts
 * @param [in] intCallback  Pointer to the callback function
 * @param [in] hDevice      Pointer to the device handler
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Registers a callback function to handle ADIN1110 interrupt events in the
 *          CN0575 project, enabling asynchronous event processing.
 */
uint32_t HAL_RegisterCallback(HAL_Callback_t const *intCallback, void *hDevice);

#if defined(ADIN1110) || defined(ADIN2111)

/**
 * @brief   Registers a callback for SPI transactions
 * @param [in] spiCallback  Pointer to the callback function
 * @param [in] hDevice      Pointer to the device handler
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Registers a callback function for SPI interrupt events with the ADIN1110/2111
 *          in the CN0575 project, supporting non-blocking SPI operations.
 */
uint32_t HAL_SpiRegisterCallback(HAL_Callback_t const *spiCallback,
		void *hDevice);

/**
 * @brief   Performs an SPI read/write operation
 * @param [in]  pBufferTx  Pointer to the transmit data buffer
 * @param [out] pBufferRx  Pointer to the receive data buffer
 * @param [in]  nbBytes    Number of bytes to transfer
 * @param [in]  useDma     True to enable DMA, false to disable
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Initiates a non-blocking SPI transaction with the ADIN1110/2111 for Ethernet
 *          frame transfers in the CN0575 project, supporting lwIP integration.
 */
uint32_t HAL_SpiReadWrite(uint8_t *pBufferTx, uint8_t *pBufferRx,
		uint32_t nbBytes, bool useDma);

/**
 * @brief   Initializes the Frame Check Sequence (FCS) calculator
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Prepares the FCS calculator for Ethernet frame integrity checks in the CN0575
 *          project, ensuring data reliability over SPE.
 */
uint32_t HAL_FcsInit(void);

/**
 * @brief   Uninitializes the Frame Check Sequence (FCS) calculator
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Releases resources used by the FCS calculator in the CN0575 project, called
 *          during cleanup.
 */
uint32_t HAL_FcsUnInit(void);

/**
 * @brief   Calculates the Frame Check Sequence (FCS) for a data buffer
 * @param [in] pBuf     Pointer to the data buffer
 * @param [in] nbBytes  Number of bytes to calculate FCS over
 * @return  32-bit FCS value (CRC32 BZIP2)
 * @details Computes the 32-bit FCS using CRC32 BZIP2 (polynomial 0x4C11DB7, initial CRC
 *          0xFFFFFFFF, post-complemented) per IEEE 802.3, for Ethernet frames in the CN0575 project.
 */
uint32_t HAL_FcsCalculate(uint8_t *pBuf, uint32_t nbBytes);

#endif

/**
 * @brief   HAL initialization hook
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Called during driver initialization to perform platform-specific setup for the
 *          CN0575 project. Can be left empty if no additional configuration is needed.
 * @sa      HAL_UnInit_Hook()
 */
uint32_t HAL_Init_Hook(void);

/**
 * @brief   HAL uninitialization hook
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 * @details Called during driver uninitialization to perform platform-specific cleanup for
 *          the CN0575 project. Can be left empty if no additional cleanup is needed.
 * @sa      HAL_Init_Hook()
 */
uint32_t HAL_UnInit_Hook(void);

/** @} */

#else

/**
 * @name HAL Wrappers (With Device Handle)
 * @brief These macros pass the device handle and all arguments to HAL functions when
 *        ADI_HAL_USE_DEVICE_HANDLE is defined. Platform-specific prototypes must be
 *        provided in hal_port_specific.h.
 */

/** @{ */

/*!
 * @brief       Wrapper for HAL_Init_Hook() with platform-specific prototype.
 */
#define     ADI_HAL_INIT(...)                       HAL_Init_Hook(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_UnInit_Hook() with platform-specific prototype.
 */
#define     ADI_HAL_UNINIT(...)                     HAL_UnInit_Hook(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_PhyRead() with platform-specific prototype.
 */
#define     ADI_HAL_PHY_READ(...)                   HAL_PhyRead(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_PhyWrite() with platform-specific prototype.
 */
#define     ADI_HAL_PHY_WRITE(...)                  HAL_PhyWrite(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_EnableIrq() with platform-specific prototype.
 */
#define     ADI_HAL_ENABLE_IRQ(...)                 HAL_EnableIrq(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_DisableIrq() with platform-specific prototype.
 */
#define     ADI_HAL_DISABLE_IRQ(...)                HAL_DisableIrq(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_GetEnableIrq() with platform-specific prototype.
 */
#define     ADI_HAL_GET_ENABLE_IRQ(...)             HAL_GetEnableIrq(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_SetPendingIrq() with platform-specific prototype.
 */
#define     ADI_HAL_SET_PENDING_IRQ(...)            HAL_SetPendingIrq(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_GetPendingIrq() with platform-specific prototype.
 */
#define     ADI_HAL_GET_PENDING_IRQ(...)            HAL_GetPendingIrq(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_RegisterCallback() with platform-specific prototype.
 */
#define     ADI_HAL_REGISTER_CALLBACK(...)          HAL_RegisterCallback(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_SpiRegisterCallback() with platform-specific prototype.
 */
#define     ADI_HAL_SPI_REGISTER_CALLBACK(...)      HAL_SpiRegisterCallback(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_SpiReadWrite() with platform-specific prototype.
 */
#define     ADI_HAL_SPI_READ_WRITE(...)             HAL_SpiReadWrite(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_FcsInit() with platform-specific prototype.
 */
#define     ADI_HAL_FCS_INIT(...)                   HAL_FcsInit(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_FcsUnInit() with platform-specific prototype.
 */
#define     ADI_HAL_FCS_UNINIT...)                  HAL_FcsUnInit(__VA_ARGS__)

/*!
 * @brief       Wrapper for HAL_FcsCalculate() with platform-specific prototype.
 */
#define     ADI_HAL_FCS_CALCULATE(...)              HAL_FcsCalculate(__VA_ARGS__)

/** @} */

#endif

#ifdef __cplusplus
}
#endif

#endif /* HAL_H */

/** @}*/
