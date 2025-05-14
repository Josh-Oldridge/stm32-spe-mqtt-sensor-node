/**
 * @file    adin1110_enhanced_stats.c
 * @brief   Standardized Link Quality Statistics for ADIN1110 in CN0575 Project
 * @details This file implements standardized link quality statistics for the ADIN1110 Single Pair
 *          Ethernet (SPE) MAC-PHY in the CN0575 project on the STM32L496ZG-P Nucleo board.
 *          It provides metrics for auto-negotiation status, MAC packet counts, MAC statistics
 *          (TX/RX frames, errors), and PHY link quality (MSE, SQI, SNR, slicer errors) to
 *          assess 1m/50m/100m cable performance connected to a Phoenix Contact 2303-8SP1 switch.
 *          Collects metrics during ping traffic via interphase sampling, stores in arrays,
 *          averages results, and prints arrays for validation. Resets slicer counters per sample,
 *          sets slicer threshold (0x018306), checks diagnostics clock (0x1E882C), logs firmware
 *          version (0x1E0002, 0x1E0003), and reads link status (0x018302). Excludes test modes
 *          and frame generators. TDR diagnostics pending library. Logs to LPUART1 via PingTask.
 * @addtogroup adin1110 ADIN1110 Driver
 * @{
 */

#ifdef STATS

#include "adin1110_enhanced_stats.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/** @brief External variable for counting transmitted packets, incremented in lwIP_adin1110_app.c. */
extern uint32_t txIdx;

/** @brief External variable for counting received packets, incremented in lwIP_adin1110_app.c. */
extern uint32_t rxIdx;

/** @brief External PHY driver entry point, used for accessing PHY functions like GetAnStatus. */
extern adi_phy_DriverEntry_t phyDriverEntry;

/**
 * @brief Collect link quality statistics for a single sample
 * @param [in] hDevice Pointer to the ADIN1110 device handle
 * @param [out] sample Pointer to store the collected metrics
 * @details Resets slicer counters, reads MSE, slicer errors, and calculates SQI, SNR,
 *          and link quality during ping traffic. Stores results in the provided sample
 *          struct. Called by PingTask every 100 pings.
 * @return adi_eth_Result_e Result code
 */
adi_eth_Result_e collectLinkQualityStats(adin1110_DeviceHandle_t *hDevice, LinkQualitySample *sample) {
    adi_eth_Result_e result;
    adi_phy_MseLinkQuality_t mseQuality = {0};
    uint16_t slicerMaxAbsVal = 0, slicerSpikeCnt = 0, spikeCntrlVal = 0, maxAbsCntrlVal = 0;

    if (hDevice == NULL || *hDevice == NULL || sample == NULL) {
        return ADI_ETH_INVALID_PARAM;
    }

    // Reset slicer counters
    result = adin1110_PhyWrite(*hDevice, 0x01800E, 0x0000); // SPIKE_CNTRS_CNTRL reset
    if (result != ADI_ETH_SUCCESS) return result;
    result = adin1110_PhyWrite(*hDevice, 0x01800F, 0x0000); // MAX_ABS_VALS_CNTRL reset
    if (result != ADI_ETH_SUCCESS) return result;
    result = adin1110_PhyWrite(*hDevice, 0x01800E, 0x0002); // SPIKE_CNTRS_CNTRL enable
    if (result != ADI_ETH_SUCCESS) return result;
    result = adin1110_PhyRead(*hDevice, 0x01800E, &spikeCntrlVal);
    if (result != ADI_ETH_SUCCESS) return result;
    if (spikeCntrlVal != 0x0002) return ADI_ETH_INVALID_PARAM;
    result = adin1110_PhyWrite(*hDevice, 0x01800F, 0x0002); // MAX_ABS_VALS_CNTRL enable
    if (result != ADI_ETH_SUCCESS) return result;
    result = adin1110_PhyRead(*hDevice, 0x01800F, &maxAbsCntrlVal);
    if (result != ADI_ETH_SUCCESS) return result;
    if (maxAbsCntrlVal != 0x0002) return ADI_ETH_INVALID_PARAM;

    // Read MSE
    result = adin1110_GetMseLinkQuality(*hDevice, &mseQuality);
    if (result != ADI_ETH_SUCCESS || mseQuality.mseVal == 0) {
        sample->mseVal = 0;
        sample->sqi = 0;
        sample->snrDb = 0.0f;
        sample->linkQuality = 0; // Unknown
        return result;
    }
    sample->mseVal = mseQuality.mseVal;
    sample->sqi = mseQuality.sqi;

    // Read slicer errors
    result = adin1110_PhyRead(*hDevice, 0x018308, &slicerMaxAbsVal); // SLCR_ERR_MAX_ABS_VAL
    if (result != ADI_ETH_SUCCESS) return result;
    result = adin1110_PhyRead(*hDevice, 0x018305, &slicerSpikeCnt); // SLCR_ERR_SPIKE_CNT
    if (result != ADI_ETH_SUCCESS) return result;
    sample->slicerMaxAbsError = (float)slicerMaxAbsVal / 4096.0f;
    sample->slicerSpikeCnt = slicerSpikeCnt;

    // Calculate SNR
    float noisePower = (float)mseQuality.mseVal * 1.5523f / 262144.0f;
    float mseDb = (noisePower > 0) ? 10.0f * log10f(noisePower) : -INFINITY;
    sample->snrDb = (noisePower > 0) ? -mseDb : 0.0f;

    // Map link quality (AN-2553 Table 2)
    sample->linkQuality = 0; // Unknown
    if (mseQuality.mseVal > 0x0766) {
        sample->linkQuality = 1; // Poor
    } else if (mseQuality.mseVal >= 0x05E1) {
        sample->linkQuality = 2; // Marginal
    } else if (mseQuality.mseVal > 0) {
        sample->linkQuality = 3; // Good
    }

    return ADI_ETH_SUCCESS;
}

/**
 * @brief Print enhanced statistics for the ADIN1110
 * @param [in] hDevice Pointer to the ADIN1110 device handle
 * @param [in] samples Array of link quality samples collected during ping traffic
 * @details Averages link quality metrics (MSE, SQI, SNR, slicer errors) from the provided
 *          samples, prints sample arrays for validation, and logs auto-negotiation status,
 *          MAC packet counts, and MAC statistics. Includes MSE min/max in hex, dB, and
 *          P_noise min/max. Called by PingTask in freertos.c after 5000 pings. Logs to LPUART1.
 */
void printEnhancedStats(adin1110_DeviceHandle_t *hDevice, LinkQualitySample *samples) {
    printf("printEnhancedStats: hDevice=%p, *hDevice=%p\n", hDevice, hDevice ? *hDevice : NULL);
    fflush(stdout);
    if (hDevice == NULL || *hDevice == NULL || samples == NULL) {
        printf("printEnhancedStats: Invalid handle or samples\n");
        fflush(stdout);
        return;
    }

    adi_eth_Result_e result;
    adi_eth_MacStatCounters_t macStats;
    adi_phy_AnStatus_t anStatus;
    adi_eth_LinkStatus_e linkStatus;
    bool linkUp = false, anValid = false, macValid = false, mseValid = false, slicerValid = false;
    uint16_t devId1 = 0, devId2 = 0, linkStat = 0, diagClkCtrl = 0;
    uint32_t mseSum = 0, sqiSum = 0, spikeCntSum = 0, mseCount = 0;
    float slicerErrSum = 0.0f, snrSum = 0.0f;
    uint16_t mseMin = UINT16_MAX, mseMax = 0;
    float slicerErrMin = INFINITY, slicerErrMax = 0.0f, snrMin = INFINITY, snrMax = 0.0f;
    uint32_t sqiMin = UINT32_MAX, sqiMax = 0, spikeCntMin = UINT16_MAX, spikeCntMax = 0;
    float mseDbMin = -INFINITY, mseDbMax = -INFINITY, pNoiseMin = 0.0f, pNoiseMax = 0.0f;

    // Log firmware version
    printf("Debug: Reading firmware version\n");
    result = adin1110_PhyRead(*hDevice, 0x1E0002, &devId1); // MMD1_DEV_ID1
    if (result == ADI_ETH_SUCCESS) {
        result = adin1110_PhyRead(*hDevice, 0x1E0003, &devId2); // MMD1_DEV_ID2
        if (result == ADI_ETH_SUCCESS) {
            printf("Debug: Device ID: 0x%04X, 0x%04X\n", devId1, devId2);
        } else {
            printf("Error: Failed to read MMD1_DEV_ID2 (0x1E0003): 0x%08X\n", result);
        }
    } else {
        printf("Error: Failed to read MMD1_DEV_ID1 (0x1E0002): 0x%08X\n", result);
    }

    // Check diagnostics clock
    printf("Debug: Checking diagnostics clock (0x1E882C)\n");
    result = adin1110_PhyRead(*hDevice, 0x1E882C, &diagClkCtrl);
    if (result == ADI_ETH_SUCCESS) {
        printf("Debug: Diagnostics Clock (0x1E882C): %s\n", (diagClkCtrl & 0x0001) ? "Enabled" : "Disabled");
        if (!(diagClkCtrl & 0x0001)) {
            printf("Debug: Enabling diagnostics clock (0x1E882C)\n");
            result = adin1110_PhyWrite(*hDevice, 0x1E882C, 0x0001);
            if (result != ADI_ETH_SUCCESS) {
                printf("Error: Failed to enable diagnostics clock (0x1E882C): 0x%08X\n", result);
            }
        }
    } else {
        printf("Error: Failed to read diagnostics clock (0x1E882C): 0x%08X\n", result);
    }

    // Check link status
    printf("Debug: Checking link status\n");
    result = adin1110_GetLinkStatus(*hDevice, &linkStatus);
    if (result == ADI_ETH_SUCCESS) {
        linkUp = (linkStatus == ADI_ETH_LINK_STATUS_UP);
        printf("Debug: adin1110_GetLinkStatus = %s\n", linkUp ? "Up" : "Down");
    } else {
        printf("Error: Failed to read link status: 0x%08X\n", result);
    }

    // Read PMA link status
    printf("Debug: Reading PMA link status (0x018302)\n");
    result = adin1110_PhyRead(*hDevice, 0x018302, &linkStat); // B10L_PMA_LINK_STAT
    if (result == ADI_ETH_SUCCESS) {
        printf("Debug: PMA Link Status (0x018302): Link=%s, Descrambler=%s, LocalRx=%s, RemoteRx=%s\n",
               (linkStat & 0x0001) ? "OK" : "Not OK",
               (linkStat & 0x0010) ? "OK" : "Not OK",
               (linkStat & 0x0040) ? "OK" : "Not OK",
               (linkStat & 0x0100) ? "OK" : "Not OK");
    } else {
        printf("Error: Failed to read PMA link status (0x018302): 0x%08X\n", result);
    }

    // Read auto-negotiation status
    printf("Debug: Reading AN status\n");
    result = phyDriverEntry.GetAnStatus((*hDevice)->pPhyDevice, &anStatus);
    if (result == ADI_ETH_SUCCESS) {
        anValid = true;
        printf("Debug: AN Complete: %s, AN Link Status: %s, Tx Mode: %s\n",
               anStatus.anComplete ? "Yes" : "No",
               anStatus.anLinkStatus == ADI_PHY_LINK_STATUS_UP ? "Up" : "Down",
               anStatus.anTxMode == ADI_PHY_AN_TX_LEVEL_1P0V ? "1.0V" :
               anStatus.anTxMode == ADI_PHY_AN_TX_LEVEL_2P4V ? "2.4V" : "Unknown");
    } else {
        printf("Error: Failed to get AN status: 0x%08X\n", result);
    }

    // Read MAC statistics
    printf("Debug: Reading MAC stats\n");
    result = adin1110_GetStatCounters(*hDevice, &macStats);
    if (result == ADI_ETH_SUCCESS) {
        macValid = true;
    } else {
        printf("Error: Failed to get MAC stats: 0x%08X\n", result);
    }

    // Process samples
    printf("Debug: Processing link quality samples\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        if (samples[i].mseVal != 0) {
            mseSum += samples[i].mseVal;
            sqiSum += samples[i].sqi;
            slicerErrSum += samples[i].slicerMaxAbsError;
            spikeCntSum += samples[i].slicerSpikeCnt;
            snrSum += samples[i].snrDb;
            mseCount++;

            if (samples[i].mseVal < mseMin) mseMin = samples[i].mseVal;
            if (samples[i].mseVal > mseMax) mseMax = samples[i].mseVal;
            if (samples[i].sqi < sqiMin) sqiMin = samples[i].sqi;
            if (samples[i].sqi > sqiMax) sqiMax = samples[i].sqi;
            if (samples[i].slicerMaxAbsError < slicerErrMin) slicerErrMin = samples[i].slicerMaxAbsError;
            if (samples[i].slicerMaxAbsError > slicerErrMax) slicerErrMax = samples[i].slicerMaxAbsError;
            if (samples[i].slicerSpikeCnt < spikeCntMin) spikeCntMin = samples[i].slicerSpikeCnt;
            if (samples[i].slicerSpikeCnt > spikeCntMax) spikeCntMax = samples[i].slicerSpikeCnt;
            if (samples[i].snrDb < snrMin) snrMin = samples[i].snrDb;
            if (samples[i].snrDb > snrMax) snrMax = samples[i].snrDb;
        }
    }

    // Calculate MSE min/max in dB and P_noise min/max
    float noisePowerMin = mseMin != UINT16_MAX ? (float)mseMin * 1.5523f / 262144.0f : 0.0f;
    float noisePowerMax = mseMax != 0 ? (float)mseMax * 1.5523f / 262144.0f : 0.0f;
    mseDbMin = (mseMin != UINT16_MAX && noisePowerMin > 0) ? 10.0f * log10f(noisePowerMin) : -INFINITY;
    mseDbMax = (mseMax != 0 && noisePowerMax > 0) ? 10.0f * log10f(noisePowerMax) : -INFINITY;
    pNoiseMin = noisePowerMin;
    pNoiseMax = noisePowerMax;

    // Print sample arrays
    printf("Debug: Link Quality Sample Arrays\n");
    printf("Sample | MSE    | SQI | SlicerErr | Spikes | SNR\n");
    printf("-------|--------|-----|-----------|--------|------\n");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        printf("%6d | 0x%04X | %3lu | %9.3f | %6u | %5.1f\n",
               i + 1, samples[i].mseVal, samples[i].sqi, samples[i].slicerMaxAbsError,
               samples[i].slicerSpikeCnt, samples[i].snrDb);
    }

    // Calculate averages
    uint16_t finalMseVal = mseCount > 0 ? (uint16_t)(mseSum / mseCount) : 0;
    uint32_t finalSqi = mseCount > 0 ? sqiSum / mseCount : 0;
    float finalSlicerErr = mseCount > 0 ? slicerErrSum / mseCount : 0.0f;
    uint16_t finalSpikeCnt = mseCount > 0 ? (uint16_t)(spikeCntSum / mseCount) : 0;
    float finalSnrDb = mseCount > 0 ? snrSum / mseCount : 0.0f;
    mseValid = mseCount > 0;
    slicerValid = mseCount > 0 && finalSlicerErr < 0.5f;

    // Calculate noise power and MSE dB for average
    float noisePower = mseValid ? (float)finalMseVal * 1.5523f / 262144.0f : 0.0f;
    float mseDb = (mseValid && noisePower > 0) ? 10.0f * log10f(noisePower) : -INFINITY;

    // Map link quality (AN-2553 Table 2)
    const char *linkQualityStr = mseValid ? "Unknown" : "N/A";
    if (mseValid) {
        if (finalMseVal > 0x0766) {
            linkQualityStr = "Poor (>0x0766, SNR < 19.5 dB, Est. BER > 10^-8)";
        } else if (finalMseVal >= 0x05E1) {
            linkQualityStr = "Marginal (0x05E1–0x0766, SNR 19.5–20.5 dB, Est. BER 10^-8 to 10^-10)";
        } else {
            linkQualityStr = "Good (<0x05E1, SNR > 20.5 dB, Est. BER < 10^-10)";
        }
    }

    // Map BER (AN-2553 Table 3)
    const char *berStr = mseValid ? "Unknown" : "N/A";
    if (mseValid) {
        if (finalMseVal < 0x02A0) {
            berStr = "<10^-14";
        } else if (finalMseVal <= 0x034E) {
            berStr = "<10^-14";
        } else if (finalMseVal <= 0x0429) {
            berStr = "<10^-14";
        } else if (finalMseVal <= 0x053D) {
            berStr = "10^-14 to 10^-11";
        } else if (finalMseVal <= 0x0698) {
            berStr = "10^-11 to 10^-9";
        } else if (finalMseVal <= 0x084E) {
            berStr = "10^-9 to 10^-7";
        } else if (finalMseVal <= 0x0A74) {
            berStr = ">10^-7";
        } else {
            berStr = ">10^-7";
        }
    }

    // Color-coded link quality (AN-2553 Table 5)
    const char *colorStr = slicerValid ? "Unknown" : "N/A";
    if (slicerValid) {
        if (finalSpikeCnt > 0 && finalSlicerErr >= 0.5f) {
            colorStr = "Red (Poor)";
        } else if (finalSpikeCnt > 0 && finalSlicerErr >= 0.3125f) {
            colorStr = "Yellow (Marginal)";
        } else {
            colorStr = "Green (Good)";
        }
    }

    // Print statistics
    printf("=== ADIN1110 Link Quality Stats ===\n");
    printf("Link Status: %s\n", linkUp ? "Up" : "Down");
    printf("Auto-Negotiation Status:\n");
    printf("  Complete: %s, Link Status: %s, Tx Mode: %s\n",
           anValid ? (anStatus.anComplete ? "Yes" : "No") : "N/A",
           anValid ? (anStatus.anLinkStatus == ADI_PHY_LINK_STATUS_UP ? "Up" : "Down") : "N/A",
           anValid ? (anStatus.anTxMode == ADI_PHY_AN_TX_LEVEL_1P0V ? "1.0V" :
                      anStatus.anTxMode == ADI_PHY_AN_TX_LEVEL_2P4V ? "2.4V" : "Unknown") : "N/A");
    printf("MAC Packet Counts:\n");
    printf("  Total Sent Packets: %lu\n", txIdx);
    printf("  Total Received Packets: %lu\n", rxIdx);
    printf("MAC Statistics:\n");
    if (macValid) {
        uint32_t totalRxErrors = macStats.RX_CRC_ERR_CNT + macStats.RX_ALGN_ERR_CNT +
                                 macStats.RX_LS_ERR_CNT + macStats.RX_PHY_ERR_CNT;
        printf("  TX Frames: %lu, RX Frames: %lu\n", macStats.TX_FRM_CNT, macStats.RX_FRM_CNT);
        printf("  Total RX Errors: %lu (CRC: %lu, Alignment: %lu, Length: %lu, PHY: %lu)\n",
               totalRxErrors, macStats.RX_CRC_ERR_CNT, macStats.RX_ALGN_ERR_CNT,
               macStats.RX_LS_ERR_CNT, macStats.RX_PHY_ERR_CNT);
        printf("  RX Drops (Full): %lu, RX Drops (Filtered): %lu\n",
               macStats.RX_DROP_FULL_CNT, macStats.RX_DROP_FILT_CNT);
        printf("  RX Broadcast: %lu, RX Multicast: %lu, RX Unicast: %lu\n",
               macStats.RX_BCAST_CNT, macStats.RX_MCAST_CNT, macStats.RX_UCAST_CNT);
    } else {
        printf("  MAC stats unavailable\n");
    }
    printf("PHY Link Quality:\n");
    printf("  MSE: %s0x%04X (MSE_dB=%.1f, P_noise=%.6f)\n", mseValid ? "" : "N/A ", finalMseVal, mseDb, noisePower);
    printf("  MSE Min/Max: 0x%04X / 0x%04X (MSE_dB=%.1f / %.1f, P_noise=%.6f / %.6f)\n",
           mseMin, mseMax, mseDbMin, mseDbMax, pNoiseMin, pNoiseMax);
    printf("  Link Quality: %s\n", linkQualityStr);
    printf("  SQI: %lu (7=Best, 0=Worst, Est. BER: %s)\n", finalSqi, berStr);
    printf("  SQI Min/Max: %lu / %lu\n", sqiMin, sqiMax);
    printf("  SNR: %s%.1f dB\n", mseValid ? "" : "N/A ", finalSnrDb);
    printf("  SNR Min/Max: %.1f / %.1f dB\n", snrMin, snrMax);
    printf("  Slicer Max Abs Error: %s%.3f symbol units (Threshold: 0.5)\n", slicerValid ? "" : "N/A ", finalSlicerErr);
    printf("  Slicer Max Abs Error Min/Max: %.3f / %.3f\n", slicerErrMin, slicerErrMax);
    printf("  Slicer Error Spikes: %s%u (Threshold: 0.3125)\n", slicerValid ? "" : "N/A ", finalSpikeCnt);
    printf("  Slicer Spikes Min/Max: %lu / %lu\n", spikeCntMin, spikeCntMax);
    printf("  Overall Link Quality: %s\n", colorStr);
    printf("TDR Diagnostics: Pending library integration\n");
    fflush(stdout);
}

/**
 * @}
 */

#endif /* STATS */
