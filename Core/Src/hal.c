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
 * @file    hal.c
 * @brief   Hardware Abstraction Layer (HAL) Implementation for STM32L496ZG-P
 * @details Implements low-level hardware functions for the STM32L496ZG-P Nucleo board
 *          in the CN0575 SPE board project. Provides IRQ control, SPI communication with
 *          the ADIN1110 MAC-PHY, and optional MDIO access for PHY operations. Supports
 *          integration with lwIP and ADIN1110 drivers for Ethernet communication.
 */

/** @addtogroup hal HAL API
 *  @{
 */

#include <string.h>
#include "hal.h"
#include "hal_port_specific.h"

#ifdef MDIO_GPIO
#include "../../mdio_gpio/mdio_gpio.h"
#endif

#ifdef ADIN1100
/**
 * @brief   MDIO Read Clause 45
 * @details Reads a 16-bit value from a Clause 45 PHY register using either GPIO-based
 *          MDIO (if MDIO_GPIO defined) or an ADI-specific MDIO function. Used for PHY
 *          register access, though not directly applicable to ADIN1110 (SPI-based).
 * @param [in] hwAddr    Hardware PHY address
 * @param [in] RegAddr   Register address (DEVTYPE + ADDR)
 * @param [out] data     Pointer to store the read value
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 */
uint32_t HAL_PhyRead(uint8_t hwAddr, uint32_t RegAddr, uint16_t *data)
{
#ifdef MDIO_GPIO
    return (uint32_t)mdioGPIORead_cl45(hwAddr, RegAddr, data);
#else
    return (uint32_t)adi_MdioRead_Cl45(hwAddr, RegAddr, data);
#endif
}

/**
 * @brief   MDIO Write Clause 45
 * @details Writes a 16-bit value to a Clause 45 PHY register using either GPIO-based
 *          MDIO or an ADI-specific function. Not used by ADIN1110 (SPI-based) in current
 *          project context.
 * @param [in] hwAddr    Hardware PHY address
 * @param [in] RegAddr   Register address (DEVTYPE + ADDR)
 * @param [in] data      Value to write
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 */
uint32_t HAL_PhyWrite(uint8_t hwAddr, uint32_t RegAddr, uint16_t data)
{
#ifdef MDIO_GPIO
  return mdioGPIOWrite_cl45(hwAddr, RegAddr, data);
#else
  return adi_MdioWrite_Cl45(hwAddr, RegAddr, data);
#endif
}
#endif

/**
 * @brief   Disable IRQ for ADIN1110
 * @details Calls HAL_INT_N_DisableIRQ() to disable the interrupt line, mapped to EXTI15_10_IRQn.
 * @return  ADI_HAL_SUCCESS
 */
uint32_t HAL_DisableIrq(void)
{
    HAL_INT_N_DisableIRQ();

    return ADI_HAL_SUCCESS;
}

/**
 * @brief   Enable IRQ for ADIN1110
 * @details Calls HAL_INT_N_EnableIRQ() to enable the interrupt line, mapped to EXTI15_10_IRQn.
 * @return  ADI_HAL_SUCCESS
 */
uint32_t HAL_EnableIrq(void)
{
    HAL_INT_N_EnableIRQ();

    return ADI_HAL_SUCCESS;
}

/**
 * @brief   Set IRQ pending state
 * @details Sets the EXTI15_10_IRQn interrupt as pending using NVIC, typically for testing.
 * @return  ADI_HAL_SUCCESS
 */
uint32_t HAL_SetPendingIrq(void)
{
    NVIC_SetPendingIRQ(EXTI15_10_IRQn);

    return ADI_HAL_SUCCESS;
}

/**
 * @brief   Get IRQ pending state
 * @details Checks if EXTI15_10_IRQn is pending via NVIC_GetPendingIRQ().
 * @return  Pending status (1 if pending, 0 otherwise)
 */
uint32_t HAL_GetPendingIrq(void)
{
    return NVIC_GetPendingIRQ(EXTI15_10_IRQn);
}

/**
 * @brief   Get IRQ enable state
 * @details Checks if EXTI15_10_IRQn is enabled via NVIC_GetEnableIRQ().
 * @return  Enable status (1 if enabled, 0 otherwise)
 */
uint32_t HAL_GetEnableIrq(void)
{
    return NVIC_GetEnableIRQ(EXTI15_10_IRQn);
}

/**
 * @brief   Register PHY IRQ callback function
 * @details Delegates to BSP_RegisterIRQCallback() to register an interrupt callback for
 *          ADIN1110 events, passing the device handle.
 * @param [in] intCallback  Callback function to register
 * @param [in] hDevice      Device handle
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 */
uint32_t HAL_RegisterCallback(HAL_Callback_t const *intCallback, void * hDevice)
{
    return BSP_RegisterIRQCallback (intCallback, hDevice);
}

/**
 * @brief   SPI write/read operation for ADIN1110
 * @details Calls BSP_spi2_write_and_read() to perform an SPI transaction with the ADIN1110,
 *          supporting both DMA and blocking modes. Used for Ethernet frame transfers.
 * @param [in]  pBufferTx  Transmit buffer
 * @param [out] pBufferRx  Receive buffer
 * @param [in]  nbBytes    Number of bytes to transfer
 * @param [in]  useDma     Enable DMA if true
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 */
uint32_t HAL_SpiReadWrite(uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes, bool useDma)
{
    return BSP_spi2_write_and_read (pBufferTx, pBufferRx, nbBytes, useDma);
}


/**
 * @brief   Register SPI callback function for ADIN1110
 * @details Delegates to BSP_spi2_register_callback() to set up an SPI interrupt callback,
 *          used for handling ADIN1110 SPI transaction completion.
 * @param [in] spiCallback  Callback function to register
 * @param [in] hDevice      Device handle
 * @return  ADI_HAL_SUCCESS on success, ADI_HAL_ERROR on failure
 */
uint32_t HAL_SpiRegisterCallback(HAL_Callback_t const *spiCallback, void * hDevice)
{
    return BSP_spi2_register_callback (spiCallback, hDevice);
}

/**
 * @brief   HAL initialization hook
 * @details Empty implementation returning success; intended for platform-specific init.
 * @note    Can be extended for custom initialization if needed.
 * @return  ADI_HAL_SUCCESS
 */
uint32_t HAL_Init_Hook(void)
{
    return ADI_HAL_SUCCESS;
}

/**
 * @brief   HAL uninitialization hook
 * @details Empty implementation returning success; intended for platform-specific cleanup.
 * @note    Can be extended for custom cleanup if needed.
 * @return  ADI_HAL_SUCCESS
 */
uint32_t HAL_UnInit_Hook(void)
{
    return ADI_HAL_SUCCESS;
}

/** @}*/
