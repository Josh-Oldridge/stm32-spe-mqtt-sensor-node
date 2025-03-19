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
 * @file    adin1110.c
 * @brief   Implementation of the ADIN1110 MAC-PHY Software Driver.
 * @details Contains the driver logic for the ADIN1110 MAC-PHY, interfacing with the
 *          STM32L496ZG-P Nucleo board via SPI (OPEN Alliance or Generic) to manage
 *          10BASE-T1L Ethernet communication on the CN0575 SPE board. Integrates MAC
 *          and PHY functionality for secure MQTT transmission of sensor data over
 *          TLSv1.2 using lwIP and mbedtls. Wraps lower-level MAC and PHY driver calls.
 */

/** @addtogroup adin1110 ADIN1110 MAC-PHY Software Driver
 *  @{
 */

#include "adin1110.h"
#include "hal.h"
#include "adi_mac.h"
#include "adi_phy.h"
#include "adi_eth_common.h"
#include "ADIN1110_mac_addr_rdef.h"

/*! @cond PRIVATE */

/** @brief Global device handle for internal use in PHY read/write callbacks. */
adin1110_DeviceHandle_t pDeviceHandle;

/**
 * @brief Write to a PHY register via the MAC driver.
 * @param [in] hwAddr PHY hardware address (fixed at ADIN1110_PHY_ADDR).
 * @param [in] regAddr PHY register address (32-bit, Clause 45).
 * @param [in] data 16-bit value to write.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Internal callback for PHY driver to write registers via the ADIN1110 MAC’s SPI-to-MDIO bridge.
 */
static uint32_t PhyWrite(uint8_t hwAddr, uint32_t regAddr, uint16_t data)
{
    return (uint32_t)macDriverEntry.PhyWrite(pDeviceHandle->pMacDevice , hwAddr, regAddr, data);
}

/**
 * @brief Read from a PHY register via the MAC driver.
 * @param [in] hwAddr PHY hardware address (fixed at ADIN1110_PHY_ADDR).
 * @param [in] regAddr PHY register address (32-bit, Clause 45).
 * @param [out] data Pointer to store the 16-bit register value.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Internal callback for PHY driver to read registers via the ADIN1110 MAC’s SPI-to-MDIO bridge.
 */
static uint32_t PhyRead(uint8_t hwAddr, uint32_t regAddr, uint16_t *data)
{
    return (uint32_t)macDriverEntry.PhyRead(pDeviceHandle->pMacDevice , hwAddr, regAddr, data);
}

/*! @endcond */

/*
 * @brief ADIN1110 driver initialization.
 * @param [out] hDevice Pointer to store the device handle.
 * @param [in] pCfg Pointer to device configuration structure.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Initializes the ADIN1110 MAC and PHY drivers over SPI with the provided
 *          configuration. Allocates memory for both MAC and PHY instances within the
 *          provided buffer. Leaves the PHY in software powerdown state; call
 *          adin1110_Enable() to activate the link.
 * @sa adin1110_UnInit()
 */
adi_eth_Result_e adin1110_Init(adin1110_DeviceHandle_t hDevice, adin1110_DriverConfig_t *pCfg)
{
    adi_eth_Result_e        result = ADI_ETH_SUCCESS;
    adi_mac_DriverConfig_t  macDrvConfig;
    adi_phy_DriverConfig_t  phyDrvConfig;

    if (pCfg->devMemSize < ADIN1110_DEVICE_SIZE)
    {
        result = ADI_ETH_INVALID_PARAM;
        goto end;
    }

    hDevice->pUserContext = NULL;

    /* Initialize the MAC configuration structure. */
    macDrvConfig.pDevMem = (void *)pCfg->pDevMem;
    macDrvConfig.devMemSize = ADI_MAC_DEVICE_SIZE;
    macDrvConfig.fcsCheckEn = pCfg->fcsCheckEn;

    /* Initialize the PHY configuration structure. */
    phyDrvConfig.pDevMem = (void *)((uint8_t *)pCfg->pDevMem + ADI_MAC_DEVICE_SIZE);
    phyDrvConfig.devMemSize = ADI_PHY_DEVICE_SIZE;
    phyDrvConfig.enableIrq  = false;

    pDeviceHandle = hDevice;

    ADI_HAL_INIT(hDevice);

    result = macDriverEntry.Init(&hDevice->pMacDevice, &macDrvConfig, (void *)hDevice);
    if (result == ADI_ETH_SUCCESS)
    {
        /* PHY address, used in internal MDIO accesses, is fixed in hardware. */
        phyDrvConfig.addr = ADIN1110_PHY_ADDR;
        result = phyDriverEntry.Init(&hDevice->pPhyDevice, &phyDrvConfig, hDevice, PhyRead, PhyWrite);

        hDevice->pMacDevice->phyAddr = phyDrvConfig.addr;
        hDevice->pMacDevice->phyIrqMask = ADIN1110_PHY_IRQ_MASK;
    }

end:

    return result;
}

/*
 * @brief ADIN1110 driver uninitialization.
 * @param [in] hDevice Device handle.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Shuts down the ADIN1110 MAC and PHY drivers, releases hardware resources,
 *          and clears device pointers. Ensures proper cleanup via SPI.
 * @sa adin1110_Init()
 */
adi_eth_Result_e adin1110_UnInit(adin1110_DeviceHandle_t hDevice)
{
    adi_eth_Result_e    result = ADI_ETH_SUCCESS;


    result = phyDriverEntry.UnInit(hDevice->pPhyDevice);
    if (result != ADI_ETH_SUCCESS)
    {
        goto end;
    }

    result = macDriverEntry.UnInit(hDevice->pMacDevice);
    if (result != ADI_ETH_SUCCESS)
    {
        goto end;
    }

    ADI_HAL_UNINIT(hDevice);

    hDevice->pPhyDevice = NULL;
    hDevice->pMacDevice = NULL;

end:

	return result;
}

/*
 * @brief ADIN1110 driver enable.
 * @param [in] hDevice Device handle.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Brings the ADIN1110 PHY out of software powerdown via SPI, enabling MAC-PHY
 *          operation and establishing the link for Ethernet communication.
 * @sa adin1110_Disable()
 */
adi_eth_Result_e adin1110_Enable(adin1110_DeviceHandle_t hDevice)
{
    adi_eth_Result_e    result = ADI_ETH_SUCCESS;

    result = phyDriverEntry.ExitSoftwarePowerdown(hDevice->pPhyDevice);

    return result;
}

/*
 * @brief ADIN1110 driver disable.
 * @param [in] hDevice Device handle.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Places the ADIN1110 PHY into software powerdown via SPI, halting MAC-PHY
 *          operation and dropping the link.
 * @sa adin1110_Enable()
 */
adi_eth_Result_e adin1110_Disable(adin1110_DeviceHandle_t hDevice)
{
    adi_eth_Result_e    result = ADI_ETH_SUCCESS;

    result = phyDriverEntry.EnterSoftwarePowerdown(hDevice->pPhyDevice);

    return result;
}

/*
 * @brief Get device identity.
 * @param [in] hDevice Device handle.
 * @param [out] pDevId Pointer to store the device identity structure.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Reads the ADIN1110 PHY’s identification registers (PHYID1, PHYID2, DIGID0,
 *          DIGID1) via SPI to populate the device ID structure with revision, model,
 *          OUI, and package type information.
 */
adi_eth_Result_e adin1110_GetDeviceId(adin1110_DeviceHandle_t hDevice, adin1110_DeviceId_t *pDevId)
{
    adi_eth_Result_e    result = ADI_ETH_SUCCESS;
    uint16_t            val16;

    result = adin1110_PhyRead(hDevice, ADDR_MMD1_DEV_ID1, &val16);
    if (result != ADI_ETH_SUCCESS)
    {
        goto end;
    }
    pDevId->phyId = (val16 << 16);

    result = adin1110_PhyRead(hDevice, ADDR_MMD1_DEV_ID2, &val16);
    if (result != ADI_ETH_SUCCESS)
    {
        goto end;
    }
    pDevId->phyId |= val16;

    result = adin1110_PhyRead(hDevice, ADDR_MGMT_PRT_PKG, &val16);
    if (result != ADI_ETH_SUCCESS)
    {
        goto end;
    }
    pDevId->pkgType = val16;

    result = adin1110_PhyRead(hDevice, 0x1E900E, &val16);
    if (result != ADI_ETH_SUCCESS)
    {
        goto end;
    }
    pDevId->digRevNum = val16 & 0xFF;

end:
    return result;
}

/*
 * @brief ADIN1110 reset.
 * @param [in] hDevice Device handle.
 * @param [in] resetType Reset type (MAC-only or MAC+PHY).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Performs a software reset of the ADIN1110 MAC and optionally the PHY via SPI.
 *          A MAC+PHY reset reinitializes the PHY, while a MAC-only reset clears MAC settings
 *          and interrupt masks, requiring reconfiguration and a call to adin1110_SyncConfig().
 * @sa adin1110_SyncConfig()
 */
adi_eth_Result_e adin1110_Reset(adin1110_DeviceHandle_t hDevice, adi_eth_ResetType_e resetType)
{
    adi_eth_Result_e    result = ADI_ETH_SUCCESS;

    result = macDriverEntry.Reset(hDevice->pMacDevice, resetType);

    /* Re-initialize the PHY if needed */
    if ((result == ADI_ETH_SUCCESS) && (resetType == ADI_ETH_RESET_TYPE_MAC_PHY))
    {
        result = phyDriverEntry.ReInitPhy(hDevice->pPhyDevice);
    }

    return result;
}

/*
 * @brief ADIN1110 configuration synchronization.
 * @param [in] hDevice Device handle.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Synchronizes the ADIN1110 MAC configuration via SPI, setting configSync to true
 *          and enabling interrupts. For OA SPI, sets CONFIG0.SYNC to allow frame transmission.
 *          Prevents further configuration changes until reset or reinitialization.
 * @sa adin1110_Reset()
 */
adi_eth_Result_e adin1110_SyncConfig(adin1110_DeviceHandle_t hDevice)
{
    return macDriverEntry.SyncConfig(hDevice->pMacDevice);
}

/*
 * @brief Read link status.
 * @param [in] hDevice Device handle.
 * @param [out] linkStatus Pointer to store the link status (UP or DOWN).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Retrieves the ADIN1110 MAC’s link status from the STATUS1 register via SPI,
 *          reflecting the PHY’s link state without direct PHY register access.
 */
adi_eth_Result_e adin1110_GetLinkStatus(adin1110_DeviceHandle_t hDevice, adi_eth_LinkStatus_e *linkStatus)
{
    return macDriverEntry.GetLinkStatus(hDevice->pMacDevice, 0, linkStatus);

}

/*
 * @brief Read MAC statistics counters.
 * @param [in] hDevice Device handle.
 * @param [out] stat Pointer to store the MAC statistics counters.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Reads the ADIN1110 MAC’s statistics counters (e.g., frame counts, errors) via SPI
 *          for Port 1, providing insight into Ethernet traffic and performance.
 */
adi_eth_Result_e adin1110_GetStatCounters(adin1110_DeviceHandle_t hDevice, adi_eth_MacStatCounters_t *stat)
{
    return macDriverEntry.GetStatCounters(hDevice->pMacDevice, 1, stat);
}

/*
 * @brief Enable/disable the status LED.
 * @param [in] hDevice Device handle.
 * @param [in] enable True to enable, false to disable the status LED.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Controls the ADIN1110 PHY’s LED_0 (fixed in hardware) via SPI, used to indicate
 *          link or activity status on the CN0575 SPE board.
 */
adi_eth_Result_e adin1110_LedEn(adin1110_DeviceHandle_t hDevice, bool enable)
{
  return phyDriverEntry.LedEn(hDevice->pPhyDevice, ADI_PHY_LED_0,  enable);
}

/*
 * @brief Set loopback mode.
 * @param [in] hDevice Device handle.
 * @param [in] loopbackMode Loopback mode (e.g., PCS, PMA, MACIF).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Configures the ADIN1110 PHY’s loopback mode via SPI for diagnostic testing,
 *          supporting various modes like PCS, PMA, or MACIF. Set to NONE for normal operation.
 */
adi_eth_Result_e adin1110_SetLoopbackMode(adin1110_DeviceHandle_t hDevice, adi_phy_LoopbackMode_e loopbackMode)
{
    return phyDriverEntry.SetLoopbackMode(hDevice->pPhyDevice, loopbackMode);
}

/*
 * @brief Set test mode.
 * @param [in] hDevice Device handle.
 * @param [in] testMode Test mode (e.g., IEEE 802.3cg modes, TX_DISABLE).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Configures the ADIN1110 PHY’s test mode via SPI for compliance testing (e.g., IEEE
 *          802.3cg modes 1-3). Set to NONE to resume normal operation.
 */
adi_eth_Result_e adin1110_SetTestMode(adin1110_DeviceHandle_t hDevice, adi_phy_TestMode_e testMode)
{
    return phyDriverEntry.SetTestMode(hDevice->pPhyDevice, testMode);
}

/*
 * @brief Set up MAC address filter and corresponding address rules.
 * @param [in] hDevice Device handle.
 * @param [in] macAddr Pointer to 6-byte MAC address.
 * @param [in] macAddrMask Pointer to 6-byte MAC address mask (NULL for exact match).
 * @param [in] priority Priority value for the filter (e.g., TO_HOST).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Adds a MAC address filter to the ADIN1110 via SPI, configuring priority and rules
 *          (e.g., forward to host). Returns ADI_ETH_ADDRESS_FILTER_TABLE_FULL if no slots are available.
 * @sa adin1110_ClearAddressFilter()
 */
adi_eth_Result_e adin1110_AddAddressFilter(adin1110_DeviceHandle_t hDevice, uint8_t *macAddr, uint8_t *macAddrMask, uint32_t priority)
{
    adi_mac_AddressRule_t   addrRule;

    addrRule.VALUE16 = 0;
    /* Address rules only make sense if TO_HOST=1, because dropping frames is the default behaviour. */
    addrRule.TO_HOST = 1;
    addrRule.HOST_PRI = priority & (BITM_MAC_ADDR_FILT_UPR_N__HOST_PRI >> BITP_MAC_ADDR_FILT_UPR_N__HOST_PRI);
    addrRule.APPLY2PORT1 = 1;

    return macDriverEntry.AddAddressFilter(hDevice->pMacDevice, macAddr, macAddrMask, addrRule.VALUE16);
}

/*
 * @brief Clear MAC address filter entry from a specific index.
 * @param [in] hDevice Device handle.
 * @param [in] addrIndex Index of the filter entry to clear (0-15).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Frees an ADIN1110 MAC address filter entry via SPI, resetting its rules to make
 *          it available for reuse.
 * @sa adin1110_AddAddressFilter()
 */
adi_eth_Result_e adin1110_ClearAddressFilter(adin1110_DeviceHandle_t hDevice, uint32_t addrIndex)
{
    return macDriverEntry.ClearAddressFilter(hDevice->pMacDevice, addrIndex);
}

/*
 * @brief Submit Tx buffer.
 * @param [in] hDevice Device handle.
 * @param [in] pBufDesc Pointer to the buffer descriptor.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Queues a frame for transmission on the ADIN1110 MAC via SPI, setting a reference
 *          count of 1 (single-port device). Invokes the buffer’s callback when the frame is
 *          downloaded to the Tx FIFO, not necessarily transmitted successfully.
 */
adi_eth_Result_e adin1110_SubmitTxBuffer(adin1110_DeviceHandle_t hDevice, adi_eth_BufDesc_t *pBufDesc)
{
    adi_mac_FrameHeader_t   header;

    header.VALUE16 = 0x0000;
    header.EGRESS_CAPTURE = pBufDesc->egressCapt;

    /* For the ADIN1110, reference counter is always 1 */
    pBufDesc->refCount = 1;

    return macDriverEntry.SubmitTxBuffer(hDevice->pMacDevice, header, pBufDesc);
}

/*
 * @brief Submit Rx buffer to the low-priority queue.
 * @param [in] hDevice Device handle.
 * @param [in] pBufDesc Pointer to the buffer descriptor.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Queues a buffer for receiving frames in the ADIN1110 MAC’s low-priority queue via
 *          SPI. Populates the descriptor and invokes its callback upon frame reception.
 * @sa adin1110_SubmitRxBufferHp()
 */
adi_eth_Result_e adin1110_SubmitRxBuffer(adin1110_DeviceHandle_t hDevice, adi_eth_BufDesc_t *pBufDesc)
{
    return macDriverEntry.SubmitRxBuffer(hDevice->pMacDevice, pBufDesc);
}

#if defined(ADI_MAC_ENABLE_RX_QUEUE_HI_PRIO)
/*
 * @brief Submit Rx buffer to the high-priority queue.
 * @param [in] hDevice Device handle.
 * @param [in] pBufDesc Pointer to the buffer descriptor.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Queues a buffer for receiving frames in the ADIN1110 MAC’s high-priority queue via
 *          SPI, if enabled by ADI_MAC_ENABLE_RX_QUEUE_HI_PRIO. Invokes callback upon reception.
 * @sa adin1110_SubmitRxBuffer()
 */
adi_eth_Result_e adin1110_SubmitRxBufferHp(adin1110_DeviceHandle_t hDevice, adi_eth_BufDesc_t *pBufDesc)
{
    return macDriverEntry.SubmitRxBufferHp(hDevice->pMacDevice, pBufDesc);
}
#endif

#if defined(SPI_OA_EN)
/*
 * @brief Configure the chunk size used in OPEN Alliance frame transfers.
 * @param [in] hDevice Device handle.
 * @param [in] cps Chunk Payload Selector value (e.g., 8, 16, 32, 64 bytes).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Sets the OA SPI chunk size for the ADIN1110 MAC via SPI, must be called before
 *          adin1110_SyncConfig() or returns ADI_ETH_CONFIG_SYNC_ERROR.
 * @sa adin1110_GetChunkSize()
 */
adi_eth_Result_e adin1110_SetChunkSize(adin1110_DeviceHandle_t hDevice, adi_mac_OaCps_e cps)
{
    return macDriverEntry.SetChunkSize(hDevice->pMacDevice, cps);
}

/*
 * @brief Get current chunk size used in OPEN Alliance frame transfers.
 * @param [in] hDevice Device handle.
 * @param [out] pCps Pointer to store the current Chunk Payload Selector value.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Retrieves the current OA SPI chunk size setting from the ADIN1110 MAC via SPI.
 * @sa adin1110_SetChunkSize()
 */
adi_eth_Result_e adin1110_GetChunkSize(adin1110_DeviceHandle_t hDevice, adi_mac_OaCps_e *pCps)
{
    return macDriverEntry.GetChunkSize(hDevice->pMacDevice, pCps);
}
#endif

/*
 * @brief Enable or disable cut-through mode.
 * @param [in] hDevice Device handle.
 * @param [in] txcte Enable cut-through in transmit.
 * @param [in] rxcte Enable cut-through in receive (OA SPI only).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Configures cut-through mode for the ADIN1110 MAC via SPI, must be set before
 *          adin1110_SyncConfig() or returns ADI_ETH_CONFIG_SYNC_ERROR.
 * @sa adin1110_GetCutThroughMode()
 */
adi_eth_Result_e adin1110_SetCutThroughMode(adin1110_DeviceHandle_t hDevice, bool txcte, bool rxcte)
{
    return macDriverEntry.SetCutThroughMode(hDevice->pMacDevice, txcte, rxcte);
}

/*
 * @brief Get cut-through mode status.
 * @param [in] hDevice Device handle.
 * @param [out] pTxcte Pointer to store transmit cut-through status.
 * @param [out] pRxcte Pointer to store receive cut-through status.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Retrieves the current cut-through mode settings from the ADIN1110 MAC via SPI.
 * @sa adin1110_SetCutThroughMode()
 */
adi_eth_Result_e adin1110_GetCutThroughMode(adin1110_DeviceHandle_t hDevice, bool *pTxcte, bool *pRxcte)
{
    return macDriverEntry.GetCutThroughMode(hDevice->pMacDevice, pTxcte, pRxcte);
}

/*
 * @brief Set the sizes of the FIFOs.
 * @param [in] hDevice Device handle.
 * @param [in] fifoSizes FIFO size configuration structure.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Configures the ADIN1110 MAC FIFO sizes via SPI, validating total size against
 *          ADI_MAC_FIFO_MAX_SIZE. Must be set before adin1110_SyncConfig().
 * @sa adin1110_GetFifoSizes()
 */
adi_eth_Result_e adin1110_SetFifoSizes(adin1110_DeviceHandle_t hDevice, adi_mac_FifoSizes_t fifoSizes)
{
    uint32_t writeVal;
    uint32_t totalSize = (2 * (uint32_t)fifoSizes.txSize) + (2 * (uint32_t)fifoSizes.rxLoSize) + (2 * (uint32_t)fifoSizes.rxHiSize);

    if (totalSize > ADI_MAC_FIFO_MAX_SIZE)
    {
        return ADI_ETH_FIFO_SIZE_ERROR;
    }
    else
    {
        writeVal = (fifoSizes.txSize << BITP_MAC_FIFO_SIZE_HTX_SIZE) |
                (fifoSizes.rxLoSize << BITP_MAC_FIFO_SIZE_P1_RX_LO_SIZE) |
                (fifoSizes.rxHiSize << BITP_MAC_FIFO_SIZE_P1_RX_HI_SIZE);

        return macDriverEntry.SetFifoSizes(hDevice->pMacDevice, writeVal);
    }
}

/*
 * @brief Get the current sizes of the FIFOs.
 * @param [in] hDevice Device handle.
 * @param [out] pFifoSizes Pointer to store the FIFO size configuration.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Retrieves the current ADIN1110 MAC FIFO sizes (Tx, Rx low/high) via SPI.
 * @sa adin1110_SetFifoSizes()
 */
adi_eth_Result_e adin1110_GetFifoSizes(adin1110_DeviceHandle_t hDevice, adi_mac_FifoSizes_t *pFifoSizes)
{
    adi_eth_Result_e    result  = ADI_ETH_SUCCESS;
    uint32_t            readVal;

    result = macDriverEntry.GetFifoSizes(hDevice->pMacDevice, &readVal);
    pFifoSizes->txSize    = (adi_mac_HtxFifoSize_e)((readVal & BITM_MAC_FIFO_SIZE_HTX_SIZE) >> BITP_MAC_FIFO_SIZE_HTX_SIZE);
    pFifoSizes->rxLoSize  = (adi_mac_RxFifoSize_e)((readVal & BITM_MAC_FIFO_SIZE_P1_RX_LO_SIZE) >> BITP_MAC_FIFO_SIZE_P1_RX_LO_SIZE);
    pFifoSizes->rxHiSize  = (adi_mac_RxFifoSize_e)((readVal & BITM_MAC_FIFO_SIZE_P1_RX_HI_SIZE) >> BITP_MAC_FIFO_SIZE_P1_RX_HI_SIZE);
    return result;
}

/*
 * @brief Clear receive and/or transmit FIFOs.
 * @param [in] hDevice Device handle.
 * @param [in] clearMode FIFO clear mode (e.g., Rx, Tx, all).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Clears specified ADIN1110 MAC FIFOs via SPI, supporting multiple clear options
 *          combined with OR operations.
 */
adi_eth_Result_e adin1110_ClearFifos(adin1110_DeviceHandle_t hDevice, adi_mac_FifoClrMode_e clearMode)
{
    return macDriverEntry.ClearFifos(hDevice->pMacDevice, clearMode);
}

/*
 * @brief Enable/disable promiscuous mode.
 * @param [in] hDevice Device handle.
 * @param [in] bFlag True to enable, false to disable promiscuous mode.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Configures the ADIN1110 MAC via SPI to forward all unmatched frames to the host,
 *          leaving existing address filters intact. Invalid frames are still dropped by hardware.
 * @sa adin1110_GetPromiscuousMode()
 */
adi_eth_Result_e adin1110_SetPromiscuousMode(adin1110_DeviceHandle_t hDevice, bool bFlag)
{
    return macDriverEntry.SetPromiscuousMode(hDevice->pMacDevice, bFlag);
}

/*
 * @brief Get promiscuous mode status.
 * @param [in] hDevice Device handle.
 * @param [out] pFlag Pointer to store promiscuous mode status.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Retrieves the current promiscuous mode setting from the ADIN1110 MAC via SPI.
 * @sa adin1110_SetPromiscuousMode()
 */
adi_eth_Result_e adin1110_GetPromiscuousMode(adin1110_DeviceHandle_t hDevice, bool *pFlag)
{
    return macDriverEntry.GetPromiscuousMode(hDevice->pMacDevice, pFlag);
}

/*
 * @brief Enable timestamp counters and capture of receive timestamps.
 * @param [in] hDevice Device handle.
 * @param [in] format Timestamp format (e.g., 32-bit free, 1588).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Enables timestamping on the ADIN1110 MAC via SPI with the specified format,
 *          must be called before adin1110_SyncConfig() or returns ADI_ETH_CONFIG_SYNC_ERROR.
 */
adi_eth_Result_e adin1110_TsEnable(adin1110_DeviceHandle_t hDevice, adi_mac_TsFormat_e format)
{
    return macDriverEntry.TsEnable(hDevice->pMacDevice, format);
}

/*
 * @brief Synchronously clear all timestamp counters.
 * @param [in] hDevice Device handle.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Resets all ADIN1110 MAC timestamp counters to zero via SPI synchronously.
 */
adi_eth_Result_e adin1110_TsClear(adin1110_DeviceHandle_t hDevice)
{
    return macDriverEntry.TsClear(hDevice->pMacDevice);
}

/*
 * @brief Configure and start TS_TIMER waveform generation.
 * @param [in] hDevice Device handle.
 * @param [in] pTimerConfig Pointer to timer configuration structure.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Configures and starts the ADIN1110 MAC’s TS_TIMER via SPI with period, duty cycle,
 *          idle state, and start time. Requires prior adin1110_TsEnable() call.
 * @sa adin1110_TsEnable(), adin1110_TsTimerStop()
 */
adi_eth_Result_e adin1110_TsTimerStart(adin1110_DeviceHandle_t hDevice, adi_mac_TsTimerConfig_t *pTimerConfig)
{
    return macDriverEntry.TsTimerStart(hDevice->pMacDevice, pTimerConfig);
}

/*
 * @brief Halt the TS_TIMER waveform generation.
 * @param [in] hDevice Device handle.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Stops the ADIN1110 MAC’s TS_TIMER waveform generation via SPI.
 * @sa adin1110_TsTimerStart()
 */
adi_eth_Result_e adin1110_TsTimerStop(adin1110_DeviceHandle_t hDevice)
{
    return macDriverEntry.TsTimerStop(hDevice->pMacDevice);
}

/*
 * @brief Set the internal seconds and nanoseconds counters to the given values.
 * @param [in] hDevice Device handle.
 * @param [in] seconds Seconds value to set.
 * @param [in] nanoseconds Nanoseconds value to set.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Sets the ADIN1110 MAC’s timestamp counters via SPI, coercing nanoseconds to
 *          counter limits. Requires prior adin1110_TsEnable() call.
 * @sa adin1110_TsEnable(), adin1110_TsSyncClock()
 */
adi_eth_Result_e adin1110_TsSetTimerAbsolute(adin1110_DeviceHandle_t hDevice, uint32_t seconds, uint32_t nanoseconds)
{
    return macDriverEntry.TsSetTimerAbsolute(hDevice->pMacDevice, seconds, nanoseconds);
}

/*
 * @brief Calculate and adjust the counter accumulator addend to adjust its frequency.
 * @param [in] hDevice Device handle.
 * @param [in] tError Time difference to correct (reference - local).
 * @param [in] referenceTimeNsDiff Reference time difference in nanoseconds.
 * @param [in] localTimeNsDiff Local time difference in nanoseconds.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Fine-tunes the ADIN1110 MAC’s timestamp clock frequency via SPI based on time
 *          error and differences. Requires prior adin1110_TsEnable() call.
 * @sa adin1110_TsEnable(), adin1110_TsSetTimerAbsolute()
 */
adi_eth_Result_e adin1110_TsSyncClock(adin1110_DeviceHandle_t hDevice, int64_t tError, uint64_t referenceTimeNsDiff, uint64_t localTimeNsDiff)
{
    return macDriverEntry.TsSyncClock(hDevice->pMacDevice, tError, referenceTimeNsDiff, localTimeNsDiff);
}

/*
 * @brief Retrieve and parse the TS_EXT_CAPT timestamp.
 * @param [in] hDevice Device handle.
 * @param [out] pCapturedTimespec Pointer to store the parsed timestamp.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Reads the ADIN1110 MAC’s external capture timestamp (TS_EXT_CAPT) via SPI,
 *          triggered by TS_CAPT pin assertion, using the format set by adin1110_TsEnable().
 * @sa adin1110_TsEnable(), adin1110_TsConvert()
 */
adi_eth_Result_e adin1110_TsGetExtCaptTimestamp(adin1110_DeviceHandle_t hDevice, adi_mac_TsTimespec_t *pCapturedTimespec)
{
    return macDriverEntry.TsGetExtCaptTimestamp(hDevice->pMacDevice, pCapturedTimespec);
}

/*
 * @brief Retrieve and parse the TTSC* transmit timestamp.
 * @param [in] hDevice Device handle.
 * @param [in] egressReg Egress capture register (A, B, or C).
 * @param [out] pCapturedTimespec Pointer to store the parsed timestamp.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Reads an ADIN1110 MAC egress timestamp (TTSCA/B/C) via SPI, captured based on
 *          frame header settings, using the format from adin1110_TsEnable().
 * @sa adin1110_TsEnable(), adin1110_TsConvert()
 */
adi_eth_Result_e adin1110_TsGetEgressTimestamp(adin1110_DeviceHandle_t hDevice, adi_mac_EgressCapture_e egressReg, adi_mac_TsTimespec_t *pCapturedTimespec)
{
    return macDriverEntry.TsGetEgressTimestamp(hDevice->pMacDevice, egressReg, pCapturedTimespec);
}

/*
 * @brief Parse a timestamp in a specific format.
 * @param [in] timestampLowWord Lower 32 bits of timestamp.
 * @param [in] timestampHighWord Upper 32 bits of timestamp (0 for 32-bit formats).
 * @param [in] format Timestamp format to parse (e.g., 32-bit free, 1588).
 * @param [out] pTimespec Pointer to store the parsed timespec.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Converts raw ADIN1110 timestamp values to a timespec structure (seconds,
 *          nanoseconds) based on the specified format.
 */
adi_eth_Result_e adin1110_TsConvert(uint32_t timestampLowWord, uint32_t timestampHighWord, adi_mac_TsFormat_e format, adi_mac_TsTimespec_t *pTimespec)
{
    return macDriverEntry.TsConvert(timestampLowWord, timestampHighWord, format, pTimespec);
}

/*
 * @brief Calculate the difference between two parsed timestamps in nanoseconds.
 * @param [in] pTsA First timestamp (minuend).
 * @param [in] pTsB Second timestamp (subtrahend).
 * @return Difference in nanoseconds (TsA - TsB).
 * @details Computes the nanosecond difference between two ADIN1110 timestamps, returning
 *          a negative value if TsB exceeds TsA.
 */
int64_t adin1110_TsSubtract(adi_mac_TsTimespec_t *pTsA, adi_mac_TsTimespec_t *pTsB)
{
    return macDriverEntry.TsSubtract(pTsA, pTsB);
}

/*
 * @brief Register callback for driver events.
 * @param [in] hDevice Device handle.
 * @param [in] cbFunc Callback function to register.
 * @param [in] cbEvent Callback event (e.g., LINK_CHANGE, TIMESTAMP_RDY).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Registers a callback with the ADIN1110 MAC via SPI for interrupt-driven events,
 *          distinct from Tx/Rx buffer callbacks, to notify the application of status changes.
 */
adi_eth_Result_e adin1110_RegisterCallback(adin1110_DeviceHandle_t hDevice, adi_eth_Callback_t cbFunc, adi_mac_InterruptEvt_e cbEvent)
{
    return macDriverEntry.RegisterCallback(hDevice->pMacDevice, cbFunc, cbEvent, (void *)hDevice);
}

/*
 * @brief Set user context for the device.
 * @param [in] hDevice Device handle.
 * @param [in] pContext Pointer to user context.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Assigns a user-defined context to the ADIN1110 driver for application-specific
 *          use, not utilized by the driver itself, with validation for initialized state.
 * @sa adin1110_GetUserContext()
 */
adi_eth_Result_e adin1110_SetUserContext(adin1110_DeviceHandle_t hDevice, void *pContext)
{
    adi_eth_Result_e    result = ADI_ETH_SUCCESS;

    if (hDevice == NULL)
    {
        result = ADI_ETH_INVALID_HANDLE;
        goto end;
    }

    if ((hDevice->pPhyDevice == NULL) || (hDevice->pPhyDevice->state == ADI_PHY_STATE_UNINITIALIZED) ||
        (hDevice->pMacDevice == NULL) || (hDevice->pMacDevice->state == ADI_MAC_STATE_UNINITIALIZED))
    {
        result = ADI_ETH_DEVICE_UNINITIALIZED;
        goto end;
    }

    hDevice->pUserContext = pContext;

end:
    return result;
}

/*
 * @brief Get user context for the device.
 * @param [in] hDevice Device handle.
 * @return Pointer to user context, or NULL if uninitialized or not set.
 * @details Retrieves the user-defined context from the ADIN1110 driver, returning NULL if
 *          the device is uninitialized or no context was set.
 * @sa adin1110_SetUserContext()
 */
void *adin1110_GetUserContext(adin1110_DeviceHandle_t hDevice)
{
    void    *result = NULL;

    if (hDevice == NULL)
    {
        result = NULL;
        goto end;
    }

    if ((hDevice->pPhyDevice == NULL) || (hDevice->pPhyDevice->state == ADI_PHY_STATE_UNINITIALIZED) ||
        (hDevice->pMacDevice == NULL) || (hDevice->pMacDevice->state == ADI_MAC_STATE_UNINITIALIZED))
    {
        result = NULL;
        goto end;
    }

    result = hDevice->pUserContext;

end:
    return result;

}

/*
 * @brief Write to a MAC register.
 * @param [in] hDevice Device handle.
 * @param [in] regAddr Register address (16-bit).
 * @param [in] regData 32-bit value to write.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Writes a 32-bit value to an ADIN1110 MAC register via SPI, providing direct
 *          hardware access for advanced configuration.
 * @sa adin1110_ReadRegister()
 */
adi_eth_Result_e adin1110_WriteRegister(adin1110_DeviceHandle_t hDevice, uint16_t regAddr, uint32_t regData)
{
    return macDriverEntry.WriteRegister(hDevice->pMacDevice, regAddr, regData);
}

/*
 * @brief Read from a MAC register.
 * @param [in] hDevice Device handle.
 * @param [in] regAddr Register address (16-bit).
 * @param [out] regData Pointer to store the 32-bit register value.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Reads a 32-bit value from an ADIN1110 MAC register via SPI, allowing direct
 *          hardware status monitoring.
 * @sa adin1110_WriteRegister()
 */
adi_eth_Result_e adin1110_ReadRegister(adin1110_DeviceHandle_t hDevice, uint16_t regAddr, uint32_t *regData)
{
    return macDriverEntry.ReadRegister(hDevice->pMacDevice, regAddr, regData);
}

/*
 * @brief Write to a PHY register.
 * @param [in] hDevice Device handle.
 * @param [in] regAddr PHY register address (32-bit, Clause 45).
 * @param [in] regData 16-bit value to write.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Writes a 16-bit value to an ADIN1110 PHY register via the SPI-to-MDIO bridge,
 *          using the fixed PHY address.
 * @sa adin1110_PhyRead()
 */
adi_eth_Result_e adin1110_PhyWrite(adin1110_DeviceHandle_t hDevice, uint32_t regAddr, uint16_t regData)
{
    return macDriverEntry.PhyWrite(hDevice->pMacDevice, hDevice->pPhyDevice->phyAddr, regAddr, regData);
}

/*
 * @brief Read from a PHY register.
 * @param [in] hDevice Device handle.
 * @param [in] regAddr PHY register address (32-bit, Clause 45).
 * @param [out] regData Pointer to store the 16-bit register value.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Reads a 16-bit value from an ADIN1110 PHY register via the SPI-to-MDIO bridge,
 *          using the fixed PHY address.
 * @sa adin1110_PhyWrite()
 */
adi_eth_Result_e adin1110_PhyRead(adin1110_DeviceHandle_t hDevice, uint32_t regAddr, uint16_t *regData)
{
    return macDriverEntry.PhyRead(hDevice->pMacDevice, hDevice->pPhyDevice->phyAddr, regAddr, regData);
}


/*
 * @brief Get link quality measure based on Mean Square Error (MSE).
 * @param [in] hDevice Device handle.
 * @param [out] mseLinkQuality Pointer to store the MSE link quality metrics.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Retrieves the ADIN1110 PHY’s MSE and link quality metrics via SPI, providing
 *          signal integrity data for the 10BASE-T1L link.
 */
adi_eth_Result_e adin1110_GetMseLinkQuality(adin1110_DeviceHandle_t hDevice, adi_phy_MseLinkQuality_t *mseLinkQuality)
{
    return phyDriverEntry.GetMseLinkQuality(hDevice->pPhyDevice, mseLinkQuality);
}


/*
 * @brief Enable/disable frame generator.
 * @param [in] hDevice Device handle.
 * @param [in] enable True to enable, false to disable.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Enables or disables the ADIN1110 PHY’s frame generator via SPI for diagnostic
 *          traffic generation on the CN0575 SPE board.
 */
adi_eth_Result_e adin1110_FrameGenEn(adin1110_DeviceHandle_t hDevice, bool enable)
{
    return phyDriverEntry.FrameGenEn(hDevice->pPhyDevice, enable);
}

/*
 * @brief Set frame generator mode.
 * @param [in] hDevice Device handle.
 * @param [in] mode Frame generator mode (BURST or CONTINUOUS).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Configures the ADIN1110 PHY frame generator mode via SPI, choosing between burst
 *          or continuous frame transmission for testing.
 */
adi_eth_Result_e adin1110_FrameGenSetMode(adin1110_DeviceHandle_t hDevice, adi_phy_FrameGenMode_e mode)
{
    return phyDriverEntry.FrameGenSetMode(hDevice->pPhyDevice, mode);
}

/*
 * @brief Set frame generator frame count.
 * @param [in] hDevice Device handle.
 * @param [in] frameCnt Number of frames to generate.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Sets the ADIN1110 PHY frame generator’s frame count via SPI, controlling the
 *          number of test frames produced.
 */
adi_eth_Result_e adin1110_FrameGenSetFrameCnt(adin1110_DeviceHandle_t hDevice, uint32_t frameCnt)
{
    return phyDriverEntry.FrameGenSetFrameCnt(hDevice->pPhyDevice, frameCnt);
}

/*
 * @brief Set frame generator payload.
 * @param [in] hDevice Device handle.
 * @param [in] payload Payload type (e.g., RANDOM, 0x00).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Configures the ADIN1110 PHY frame generator’s payload pattern via SPI, supporting
 *          options like random or fixed byte values for testing.
 */
adi_eth_Result_e adin1110_FrameGenSetFramePayload(adin1110_DeviceHandle_t hDevice, adi_phy_FrameGenPayload_e payload)
{
    return phyDriverEntry.FrameGenSetFramePayload(hDevice->pPhyDevice, payload);
}

/*
 * @brief Set frame generator frame length.
 * @param [in] hDevice Device handle.
 * @param [in] frameLen Frame length in bytes (excluding overhead).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Sets the ADIN1110 PHY frame generator’s frame length via SPI, noting an additional
 *          18-byte overhead (source, destination, length, FCS). No padding for <64-byte frames.
 */
adi_eth_Result_e adin1110_FrameGenSetFrameLen(adin1110_DeviceHandle_t hDevice, uint16_t frameLen)
{
    return phyDriverEntry.FrameGenSetFrameLen(hDevice->pPhyDevice, frameLen);
}

/*
 * @brief Set frame generator interframe gap.
 * @param [in] hDevice Device handle.
 * @param [in] ifgLen Interframe gap length in bytes.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Configures the ADIN1110 PHY frame generator’s interframe gap via SPI, controlling
 *          the spacing between test frames.
 */
adi_eth_Result_e adin1110_FrameGenSetIfgLen(adin1110_DeviceHandle_t hDevice, uint16_t ifgLen)
{
    return phyDriverEntry.FrameGenSetIfgLen(hDevice->pPhyDevice, ifgLen);
}

/*
 * @brief Restart frame generator.
 * @param [in] hDevice Device handle.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Restarts the ADIN1110 PHY frame generator via SPI, clearing FG_DONE and initiating
 *          frame generation based on prior settings.
 */
adi_eth_Result_e adin1110_FrameGenRestart(adin1110_DeviceHandle_t hDevice)
{
    return phyDriverEntry.FrameGenRestart(hDevice->pPhyDevice);
}

/*
 * @brief Read frame generator status.
 * @param [in] hDevice Device handle.
 * @param [out] fgDone Pointer to store the done status (true if complete).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Checks the ADIN1110 PHY frame generator’s FG_DONE bit via SPI to determine if
 *          frame generation has completed.
 */
adi_eth_Result_e adin1110_FrameGenDone(adin1110_DeviceHandle_t hDevice, bool *fgDone)
{
    return phyDriverEntry.FrameGenDone(hDevice->pPhyDevice, fgDone);
}

/*
 * @brief Enable/disable frame checker.
 * @param [in] hDevice Device handle.
 * @param [in] enable True to enable, false to disable.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Enables or disables the ADIN1110 PHY’s frame checker via SPI for analyzing
 *          received frame integrity and errors.
 */
adi_eth_Result_e adin1110_FrameChkEn(adin1110_DeviceHandle_t hDevice, bool enable)
{
    return phyDriverEntry.FrameChkEn(hDevice->pPhyDevice, enable);
}

/*
 * @brief Select frame checker source.
 * @param [in] hDevice Device handle.
 * @param [in] source Frame checker source (PHY or MAC).
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Configures the ADIN1110 PHY frame checker’s source via SPI, choosing between PHY
 *          or MAC input for error analysis.
 */
 adi_eth_Result_e adin1110_FrameChkSourceSelect(adin1110_DeviceHandle_t hDevice, adi_phy_FrameChkSource_e source)
{
    return phyDriverEntry.FrameChkSourceSelect(hDevice->pPhyDevice, source);
}

 /*
  * @brief Read frame checker frame count.
  * @param [in] hDevice Device handle.
  * @param [out] cnt Pointer to store the frame count.
  * @return ADI_ETH_SUCCESS on success, error code otherwise.
  * @details Retrieves the ADIN1110 PHY frame checker’s frame count via SPI, latched after
  *          reading RX_CNT_ERR via adin1110_FrameChkReadRxErrCnt().
  * @sa adin1110_FrameChkReadRxErrCnt()
  */
adi_eth_Result_e adin1110_FrameChkReadFrameCnt(adin1110_DeviceHandle_t hDevice, uint32_t *cnt)
{
    return phyDriverEntry.FrameChkReadFrameCnt(hDevice->pPhyDevice, cnt);
}

/*
 * @brief Read frame checker receive errors.
 * @param [in] hDevice Device handle.
 * @param [out] cnt Pointer to store the error count.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Retrieves the ADIN1110 PHY frame checker’s receive error count via SPI, latching
 *          frame and error counters for subsequent reads.
 * @sa adin1110_FrameChkReadFrameCnt(), adin1110_FrameChkReadErrorCnt()
 */
adi_eth_Result_e adin1110_FrameChkReadRxErrCnt(adin1110_DeviceHandle_t hDevice, uint16_t *cnt)
{
    return phyDriverEntry.FrameChkReadRxErrCnt(hDevice->pPhyDevice, cnt);
}

/*
 * @brief Read frame checker error counters.
 * @param [in] hDevice Device handle.
 * @param [out] cnt Pointer to store the detailed error counters.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Retrieves detailed error counters from the ADIN1110 PHY frame checker via SPI,
 *          latched after reading RX_CNT_ERR via adin1110_FrameChkReadRxErrCnt().
 * @sa adin1110_FrameChkReadRxErrCnt()
 */
adi_eth_Result_e adin1110_FrameChkReadErrorCnt(adin1110_DeviceHandle_t hDevice, adi_phy_FrameChkErrorCounters_t *cnt)
{
    return phyDriverEntry.FrameChkReadErrorCnt(hDevice->pPhyDevice, cnt);
}

/*
 * @brief Handle ADIN1110 interrupts.
 * @param [in] hDevice Device handle.
 * @return ADI_ETH_SUCCESS on success, error code otherwise.
 * @details Processes ADIN1110 MAC interrupts via SPI, reading and clearing STATUS0/STATUS1,
 *          and invoking registered callbacks for events like link changes or Tx/Rx readiness.
 *          Typically called from an external ISR on the STM32L496ZG-P.
 */
adi_eth_Result_e adin1110_HandleInterrupt(adin1110_DeviceHandle_t hDevice)
{
    adi_eth_Result_e    result = ADI_ETH_SUCCESS;
    uint32_t            status0 = 0;
    uint32_t            status1 = 0;

    /* Validate the device handle */
    if (hDevice == NULL || hDevice->pMacDevice == NULL) {
        return ADI_ETH_INVALID_HANDLE;
    }

    /* Read STATUS0 Register */
    result = adin1110_ReadRegister(hDevice, ADDR_MAC_STATUS0, &status0);
    if (result != ADI_ETH_SUCCESS) {
        return result;
    }

    /* Read STATUS1 Register */
    result = adin1110_ReadRegister(hDevice, ADDR_MAC_STATUS1, &status1);
    if (result != ADI_ETH_SUCCESS) {
        return result;
    }

    /* Apply interrupt masks to determine active interrupts */
    uint32_t maskedStatus0 = status0 & hDevice->pMacDevice->irqMask0;
    uint32_t maskedStatus1 = status1 & hDevice->pMacDevice->irqMask1;

    /* Clear handled interrupts by writing back the masked status */
    if (maskedStatus0 != 0) {
        result = adin1110_WriteRegister(hDevice, ADDR_MAC_STATUS0, maskedStatus0);
        if (result != ADI_ETH_SUCCESS) {
            return result;
        }
    }

    if (maskedStatus1 != 0) {
        result = adin1110_WriteRegister(hDevice, ADDR_MAC_STATUS1, maskedStatus1);
        if (result != ADI_ETH_SUCCESS) {
            return result;
        }
    }

    /* Process STATUS0 Interrupts */

    /* Example: Link Status Change */
    if (maskedStatus0 & BITM_MAC_STATUS1_P1_LINK_STATUS) {
        adi_eth_LinkStatus_e linkStatus;

        /* Determine the link status */
        if (status0 & BITM_MAC_STATUS1_P1_LINK_STATUS) {
            linkStatus = ADI_ETH_LINK_STATUS_UP;
        } else {
            linkStatus = ADI_ETH_LINK_STATUS_DOWN;
        }

        /* Invoke the Link Change Callback if registered */
        if (hDevice->pMacDevice->cbFunc[ADI_MAC_EVT_LINK_CHANGE] != NULL) {
            hDevice->pMacDevice->cbFunc[ADI_MAC_EVT_LINK_CHANGE](
                hDevice->pMacDevice->cbParam[ADI_MAC_EVT_LINK_CHANGE],
                ADI_MAC_EVT_LINK_CHANGE,
                &linkStatus
            );
        }
    }

    /* Example: Link Change Event in STATUS1 */
    if (maskedStatus1 & BITM_MAC_STATUS1_LINK_CHANGE) {
        /* Determine the current link status */
        adi_eth_LinkStatus_e linkStatus;
        if (status1 & BITM_MAC_STATUS1_P1_LINK_STATUS) {
            linkStatus = ADI_ETH_LINK_STATUS_UP;
        } else {
            linkStatus = ADI_ETH_LINK_STATUS_DOWN;
        }

        /* Invoke the Link Change Callback if registered */
        if (hDevice->pMacDevice->cbFunc[ADI_MAC_EVT_LINK_CHANGE] != NULL) {
            hDevice->pMacDevice->cbFunc[ADI_MAC_EVT_LINK_CHANGE](
                hDevice->pMacDevice->cbParam[ADI_MAC_EVT_LINK_CHANGE],
                ADI_MAC_EVT_LINK_CHANGE,
                &linkStatus
            );
        }
    }

    /* Example: TX Ready Interrupt */
    if (maskedStatus1 & BITM_MAC_STATUS1_TX_RDY) {
        /* Invoke the TX Ready Callback if registered */
        if (hDevice->pMacDevice->cbFunc[ADI_MAC_EVT_TX_RDY] != NULL) {
            hDevice->pMacDevice->cbFunc[ADI_MAC_EVT_TX_RDY](
                hDevice->pMacDevice->cbParam[ADI_MAC_EVT_TX_RDY],
                ADI_MAC_EVT_TX_RDY,
                NULL
            );
        }
    }

    /* Example: RX Ready Interrupt */
    if (maskedStatus1 & BITM_MAC_STATUS1_P1_RX_RDY) {
        /* Invoke the RX Ready Callback if registered */
        if (hDevice->pMacDevice->cbFunc[ADI_MAC_EVT_P1_RX_RDY] != NULL) {
            hDevice->pMacDevice->cbFunc[ADI_MAC_EVT_P1_RX_RDY](
                hDevice->pMacDevice->cbParam[ADI_MAC_EVT_P1_RX_RDY],
                ADI_MAC_EVT_P1_RX_RDY,
                NULL
            );
        }
    }

    /* Add additional interrupt processing as needed based on your application's requirements */

    return result;
}

/** @}*/


