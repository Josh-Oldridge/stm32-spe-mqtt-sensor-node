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
 * @file    adi_spi_oa.h
 * @brief   Definitions for the OPEN Alliance SPI protocol support.
 * @details Provides structures and constants for the OPEN Alliance (OA) SPI protocol
 *          used by the ADIN1110 MAC driver on the CN0575 SPE board. Interfaces with
 *          the STM32L496ZG-P Nucleo board to manage Ethernet communication over
 *          10BASE-T1L, supporting secure MQTT transmission of sensor data via lwIP
 *          and mbedtls (TLSv1.2). Complements adi_mac.h for OA-specific SPI operations.
 */

/** @addtogroup mac ADI MAC Driver
 *  @{
 */

#ifndef ADI_SPI_OA_H
#define ADI_SPI_OA_H

#include "adi_mac.h"
#include "hal.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief SPI header size, in bytes.
 * @details Size of the control command header for OA SPI transactions (4 bytes).
 */
#define ADI_SPI_HEADER_SIZE                 (4)

/**
 * @brief Frame header size, in bytes.
 * @details Size of the frame header within OA SPI data transactions (2 bytes).
 */
#define ADI_FRAME_HEADER_SIZE               (2)

/**
 * @brief Maximum number of chunks in an OA SPI transaction (excluding 64-byte mode).
 * @details Limits the number of chunks packed into a single OA SPI transaction to 31
 *          for 8/16/32-byte chunk sizes on the ADIN1110.
 */
#define ADI_OA_MAX_CHUNK_COUNT              (31)

/**
 * @brief Maximum number of chunks in an OA SPI transaction for 64-byte mode.
 * @details Limits the number of chunks to 16 when using 64-byte chunks on the ADIN1110.
 */
#define ADI_OA_MAX_CHUNK64_COUNT            (16)

/**
 * @brief OA SPI header value indicating a parity error.
 * @details Returned by the ADIN1110 MAC when a header parity error is detected,
 *          signaling an invalid control command.
 */
#define ADI_OA_HEADER_BAD                   (0x40000000)

/**
 * @brief OA SPI Control Command Header structure.
 * @details Defines the 32-bit header format for control transactions in the OPEN
 *          Alliance SPI protocol, used by the ADIN1110 to access registers over SPI.
 */
typedef struct
{
    union {
        struct {
            uint32_t P      : 1;  /*!< Parity bit for the header (odd parity). */
            uint32_t LEN    : 7;  /*!< Length of the transaction in 32-bit words (1-128). */
            uint32_t ADDR   : 16; /*!< Register address to access (0-65535). */
            uint32_t MMS    : 4;  /*!< Memory Map Selector (0-15, selects register space). */
            uint32_t AID    : 1;  /*!< Address Increment Disable (0 = increment, 1 = fixed). */
            uint32_t WNR    : 1;  /*!< Write Not Read (0 = read, 1 = write). */
            uint32_t HDRB   : 1;  /*!< Header Bad (1 = error detected by MAC, read-only). */
            uint32_t DNC    : 1;  /*!< Do Not Care (reserved, typically 0). */
        };
        uint32_t VALUE32;         /*!< 32-bit representation of the header. */
    };
} adi_mac_OaCtrlCmdHeader_t;

/**
 * @brief OA SPI Transmit Header structure.
 * @details Defines the 32-bit header format for data transactions sent from the
 *          host to the ADIN1110 MAC over OA SPI, preceding frame data chunks.
 */
typedef struct
{
    union {
        struct {
            uint32_t P      : 1;  /*!< Parity bit for the header (odd parity). */
            uint32_t RSVD0  : 5;  /*!< Reserved bits (typically 0). */
            uint32_t TMSC   : 2;  /*!< Timestamp Chunk indicator (0-3, for 64-bit timestamps). */
            uint32_t EBO    : 6;  /*!< End Byte Offset (0-63, last byte position in chunk). */
            uint32_t EV     : 1;  /*!< End Valid (1 = last chunk of frame). */
            uint32_t RSVD1  : 1;  /*!< Reserved bit (typically 0). */
            uint32_t SWO    : 4;  /*!< Start Word Offset (0-15, first 32-bit word in chunk). */
            uint32_t SV     : 1;  /*!< Start Valid (1 = first chunk of frame). */
            uint32_t DV     : 1;  /*!< Data Valid (1 = chunk contains valid data). */
            uint32_t VS     : 2;  /*!< Vendor Specific bits (0-3, custom use). */
            uint32_t RSVD2  : 5;  /*!< Reserved bits (typically 0). */
            uint32_t NORX   : 1;  /*!< No Receive (1 = host not ready to receive data). */
            uint32_t SEQ    : 1;  /*!< Sequence bit (toggles for transaction ordering). */
            uint32_t DNC    : 1;  /*!< Do Not Care (reserved, typically 0). */
        };
        uint32_t VALUE32;         /*!< 32-bit representation of the header. */
    };
} adi_mac_OaTxHeader_t;

/**
 * @brief OA SPI Receive Footer structure.
 * @details Defines the 32-bit footer format returned by the ADIN1110 MAC over OA SPI,
 *          following data transaction chunks, providing status and metadata.
 */
typedef struct
{
    union {
        struct {
            uint32_t P      : 1;  /*!< Parity bit for the footer (odd parity). */
            uint32_t TXC    : 5;  /*!< Transmit Credits (0-31, available Tx chunks). */
            uint32_t RTSP   : 1;  /*!< Received Timestamp Parity (odd parity of timestamp). */
            uint32_t RTSA   : 1;  /*!< Received Timestamp Available (1 = timestamp present). */
            uint32_t EBO    : 6;  /*!< End Byte Offset (0-63, last byte position in chunk). */
            uint32_t EV     : 1;  /*!< End Valid (1 = last chunk of frame). */
            uint32_t FD     : 1;  /*!< Frame Dropped (1 = frame discarded by MAC). */
            uint32_t SWO    : 4;  /*!< Start Word Offset (0-15, first 32-bit word in chunk). */
            uint32_t SV     : 1;  /*!< Start Valid (1 = first chunk of frame). */
            uint32_t DV     : 1;  /*!< Data Valid (1 = chunk contains valid data). */
            uint32_t VS     : 2;  /*!< Vendor Specific bits (0-3, custom use). */
            uint32_t RCA    : 5;  /*!< Receive Chunks Available (0-31, available Rx chunks). */
            uint32_t SYNC   : 1;  /*!< Synchronized (1 = MAC config synchronized). */
            uint32_t HDRB   : 1;  /*!< Header Bad (1 = parity error in received header). */
            uint32_t EXST   : 1;  /*!< External Status (1 = external event occurred). */
        };
        uint32_t VALUE32;         /*!< 32-bit representation of the footer. */
    };
} adi_mac_OaRxFooter_t;

#ifdef __cplusplus
}
#endif

#endif /* ADI_SPI_OA_H */

/** @} */
