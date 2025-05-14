/*
 *---------------------------------------------------------------------------
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.
 * and its licensors.By using this software you agree to the terms of the
 * associated Analog Devices Software License Agreement.
 *
 *---------------------------------------------------------------------------
 */

/**
 * @file    lwIP_adin1110_app.h
 * @brief   lwIP Integration Definitions for ADIN1110 Ethernet Communication.
 * @details Defines structures, constants, and function prototypes for integrating the lwIP
 *          network stack with the ADIN1110 MAC-PHY on the STM32L496ZG-P Nucleo board in the
 *          CN0575 SPE board project. Facilitates Ethernet frame handling over SPI for secure
 *          MQTT transmission of sensor data (e.g., ADXL345 XYZ acceleration) over TLSv1.2
 *          using lwIP and mbedtls. Includes packet queue management and network interface setup.
 */

/** @addtogroup lwip_adin1110 lwIP ADIN1110 Integration
 *  @{
 */

#ifndef LWIP_ADIN1110__H
#define LWIP_ADIN1110__H

#include "adin1110.h"
#include "lwip/netif.h"
#include <lwip/etharp.h>

/**
 * @brief Flag for fixed IP configuration.
 * @details Set to 0 for DHCP, 1 for static IP (though implementation uses board_t’s flag).
 */
#define IP_FIXED 0

/**
 * @brief Maximum size of a packet queue entry (1600 bytes).
 * @details Defines the maximum Ethernet frame size including headers for queue storage.
 */
#define MAX_P_QUEUE_SZ 1600

/**
 * @brief Maximum number of entries in a packet queue (16).
 * @details Limits the queue depth for RX frame buffering.
 */
#define MAX_P_QUEUE 16

/**
 * @brief Number of packet queues (1).
 * @details Single queue used for RX frame handling in this implementation.
 */
#define MAX_PQ 1

/**
 * @brief Packet queue structure for RX frame buffering.
 * @details Manages a circular buffer of Ethernet frames received from the ADIN1110.
 */
typedef struct _pQueue {
    uint8_t pData[MAX_P_QUEUE][MAX_P_QUEUE_SZ]; /*!< Array of frame data buffers. */
    int lenData[MAX_P_QUEUE];                   /*!< Length of each frame in bytes. */
    int32_t nRdQ;                               /*!< Read index for queue dequeue. */
    int32_t nWrQ;                               /*!< Write index for queue enqueue. */
} pQueue_t;

/**
 * @brief Board configuration structure.
 * @details Holds network configuration details for the STM32L496ZG-P board.
 */
typedef struct {
    uint8_t adin1110Error;    /*!< Error flag for ADIN1110 initialization (unused in current impl). */
    uint8_t ip_addr_fixed;    /*!< 1 for static IP, 0 for DHCP. */
    uint8_t ip_addr[4];       /*!< Static IP address (e.g., {192, 168, 1, 100}). */
    uint8_t net_mask[4];      /*!< Network mask (e.g., {255, 255, 255, 0}). */
    uint8_t gateway[4];       /*!< Gateway address (e.g., {192, 168, 1, 1}). */
    uint8_t mac[6];           /*!< MAC address (unused in current impl; set via LwIP_StructInit). */
} board_t;

/**
 * @brief lwIP and ADIN1110 integration structure.
 * @details Encapsulates the ADIN1110 device handle, network interface, and MAC address.
 */
typedef struct Lwip_adin1110_s {
    adin1110_DeviceHandle_t* hDevice; /*!< Pointer to the ADIN1110 device handle. */
    struct netif netif;               /*!< lwIP network interface structure. */
    uint8_t macAddress[6];            /*!< 6-byte MAC address for the ADIN1110. */
} LwIP_ADIN1110_t;

/**
 * @brief Check if data is available in the packet queue.
 * @param [in] pQ Pointer to the queue structure.
 * @return 1 if data is available, 0 if queue is empty.
 * @details Used by lwIP to determine if RX frames are ready for processing.
 */
uint32_t pDataAvailable(pQueue_t *pQ);

/**
 * @brief Get system tick count for lwIP timing.
 * @return Current tick count in milliseconds.
 * @details Provides a timestamp for lwIP’s timing requirements, wrapping BSP_SysNow().
 */
uint32_t sys_now(void);

/**
 * @brief Discover and initialize the ADIN1110 device.
 * @param [in,out] hDevice Pointer to the ADIN1110 device handle to initialize.
 * @return 0 on success, 1 on failure.
 * @details Attempts ADIN1110 initialization with retries, setting up the driver instance.
 */
uint32_t discoveradin1110(adin1110_DeviceHandle_t *hDevice);

/**
 * @brief Initialize the lwIP ADIN1110 structure.
 * @param [in,out] eth Pointer to the lwIP ADIN1110 structure to initialize.
 * @param [in] hDevice Pointer to the ADIN1110 device handle.
 * @param [in] macAddress Pointer to the 6-byte MAC address.
 * @return ADI_ETH_SUCCESS on success, ADI_ETH_INVALID_PARAM if macAddress is NULL.
 * @details Configures the lwIP structure with the device handle and MAC address.
 */
adi_eth_Result_e LwIP_StructInit(LwIP_ADIN1110_t *eth,
		adin1110_DeviceHandle_t* hDevice, uint8_t macAddress[6]);

/**
 * @brief Initialize lwIP and ADIN1110 for network operation.
 * @param [in,out] eth Pointer to the lwIP ADIN1110 structure.
 * @param [in] boardDetails Pointer to board configuration details.
 * @details Sets up the ADIN1110, lwIP stack, and network interface with static IP or DHCP.
 */
void LwIP_Init(LwIP_ADIN1110_t *eth, board_t *boardDetails);

/**
 * @brief Link change callback for ADIN1110.
 * @param [in] pCBParam User-defined parameter (unused).
 * @param [in] Event Event identifier (unused).
 * @param [in] pArg Pointer to the new link status.
 * @details Updates GPIO and logs link state changes (UP/DOWN) for network status monitoring.
 */
void cbLinkChange(void *pCBParam, uint32_t Event, void *pArg);

/**
 * @brief Process incoming Ethernet frames for lwIP.
 * @param [in] netif Pointer to the network interface structure.
 * @return ERR_OK on success, ERR_MEM if no frame is available or on error.
 * @details Dequeues RX frames from the packet queue and passes them to lwIP’s input handler.
 */
err_t LwIP_ADIN1110LinkInput(struct netif *netif);

/**
 * @brief Global lwIP ADIN1110 connection instance.
 * @details Holds the active network configuration and ADIN1110 handle for the application.
 */
extern LwIP_ADIN1110_t myConn;

/**
 * @brief Global packet queue array for RX frame buffering.
 * @details Single queue instance used to buffer incoming Ethernet frames from ADIN1110.
 */
extern pQueue_t pQ[MAX_PQ];

/**
 * @brief Counter for total transmitted frames
 * @details Incremented for each frame sent via the ADIN1110, used for packet loss calculations
 *          in enhanced statistics.
 */
extern uint32_t txIdx;

/**
 * @brief Counter for total received frames
 * @details Incremented for each frame received via the ADIN1110, used for packet loss
 *          calculations in enhanced statistics.
 */
extern uint32_t rxIdx;
#endif /*LWIP_ADIN1110__H*/
/** @} */
