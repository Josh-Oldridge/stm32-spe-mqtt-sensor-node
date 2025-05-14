/**
 ******************************************************************************
 * @file    adin1110_enhanced_stats.h
 * @brief   Enhanced Statistics Interface for ADIN1110 in CN0575 Project
 * @details This header defines the interface for collecting enhanced statistics from the
 *          ADIN1110 Single Pair Ethernet (SPE) MAC-PHY in the CN0575 project on the
 *          STM32L496ZG-P Nucleo board. It provides functions to collect and print link
 *          quality statistics (MSE, SQI, SNR, slicer errors) during ping traffic to assess
 *          1m/50m/100m cable performance with a Phoenix Contact 2303-8SP1 switch. Includes
 *          a struct for interphase sampling and supports array-based data collection for
 *          validation, used by PingTask in freertos.c.
 * @addtogroup adin1110 ADIN1110 Driver
 * @{
 ******************************************************************************
 */

#ifdef STATS

#ifndef ADIN1110_ENHANCED_STATS_H
#define ADIN1110_ENHANCED_STATS_H

#include "adin1110.h"
#include "adi_phy.h"

/** @brief Number of samples for interphase collection */
#define NUM_SAMPLES 1000

/**
 * @brief Structure to hold link quality metrics for a single sample
 */
typedef struct {
    uint16_t mseVal;            /**< Mean Squared Error (MSE) value */
    float slicerMaxAbsError;    /**< Slicer maximum absolute error (symbol units) */
    uint16_t slicerSpikeCnt;    /**< Slicer spike count (threshold 0.3125) */
    uint32_t sqi;               /**< Signal Quality Indicator (0-7) */
    float snrDb;                /**< Signal-to-Noise Ratio (dB) */
    uint8_t linkQuality;        /**< Link quality (0=Unknown, 1=Poor, 2=Marginal, 3=Good) */
} LinkQualitySample;

/**
 * @brief Collect link quality statistics for a single sample
 * @param [in] hDevice Pointer to the ADIN1110 device handle
 * @param [out] sample Pointer to store the collected metrics
 * @details Resets slicer counters, reads MSE, slicer errors, and calculates SQI, SNR,
 *          and link quality during ping traffic. Stores results in the provided sample
 *          struct. Called by PingTask every 100 pings.
 * @return adi_eth_Result_e Result code
 */
adi_eth_Result_e collectLinkQualityStats(adin1110_DeviceHandle_t *hDevice, LinkQualitySample *sample);

/**
 * @brief Print enhanced statistics for the ADIN1110
 * @param [in] hDevice Pointer to the ADIN1110 device handle
 * @param [in] samples Array of link quality samples collected during ping traffic
 * @details Averages link quality metrics (MSE, SQI, SNR, slicer errors) from the provided
 *          samples, prints sample arrays for validation, and logs auto-negotiation status,
 *          MAC packet counts, and MAC statistics. Called by PingTask in freertos.c after
 *          1000 pings. Logs to LPUART1.
 */
void printEnhancedStats(adin1110_DeviceHandle_t *hDevice, LinkQualitySample *samples);

#endif /* ADIN1110_ENHANCED_STATS_H */

#endif /* STATS */

/**
 * @}
 */
