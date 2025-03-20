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
  * @file    boardsupport.h
  * @brief   Board Support Package (BSP) Definitions for STM32L496ZG-P Nucleo Board
  * @details This header file provides hardware abstraction functions, macros, and types for
  *          the STM32L496ZG-P Nucleo board in the CN0575 Single Pair Ethernet (SPE) board
  *          project. It supports SPI1 communication with the ADIN1110 MAC-PHY (despite being
  *          named SPI2), GPIO control for LEDs and interrupts, UART-based debug logging via
  *          LPUART1, and timing utilities. Facilitates sensor data collection from ADXL345,
  *          TMP102, and ADC1, enabling secure MQTT transmission over TLSv1.2 via lwIP when
  *          USE_LWIP is defined, or standalone Ethernet frame testing otherwise. Integrates
  *          with FreeRTOS tasks using TIM6 as the timebase source for scheduling.
  */

/** @addtogroup bsp Board Support Package
  *  @{
  */

#ifndef BOARDSUPPORT_H
#define BOARDSUPPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

/**
  * @brief Callback function type for interrupt and event handling
  * @param [in] pCBParam User-defined callback parameter
  * @param [in] Event Event identifier triggering the callback
  * @param [in] pArg Event-specific argument
  * @details Defines the signature for callbacks used in SPI transactions and IRQ handling,
  *          supporting asynchronous operations with the ADIN1110 in the CN0575 project.
  */
typedef void (* ADI_CB) (
    void      *pCBParam,
    uint32_t   Event,
    void      *pArg);

#include "stm32l4xx_hal.h"
#include "stm32l4xx_it.h"

#include "bsp_config.h"
#include "bsp_def.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"


/**
  * @brief Buffer for debug messages written to UART
  * @details Fixed-size buffer (200 bytes) for formatting and sending debug strings via UART,
  *          used by DEBUG_MESSAGE and DEBUG_RESULT macros in the CN0575 project.
  */
extern char aDebugString[200u];

/**
  * @brief SPI handle for Ethernet communication with ADIN1110
  * @details Configured SPI1 peripheral handle (despite being named hEthSpi and associated with
  *          BSP_spi2_ functions) for interfacing with the ADIN1110 MAC-PHY over SPI in the
  *          CN0575 project.
  */
extern SPI_HandleTypeDef hEthSpi;

/**
  * @brief Report a failure and halt execution
  * @param [in] FailureReason String describing the failure reason
  * @details Outputs the failure reason via UART and typically halts the system, used for
  *          error reporting in the CN0575 project.
  */
void common_Fail(char *FailureReason);

/**
  * @brief Print performance or debug information
  * @param [in] InfoString String containing debug or performance info
  * @details Sends the info string to UART for logging or monitoring purposes in the CN0575 project.
  */
void common_Perf(char *InfoString);

/**
  * @brief Macro to format and print debug messages via UART
  * @param[in] ... Variable arguments for snprintf formatting
  * @details Formats a message into aDebugString and calls common_Perf to output it, widely used
  *          for debugging in the CN0575 project.
  */
#define DEBUG_MESSAGE(...) \
  do { \
    snprintf(aDebugString, sizeof(aDebugString), __VA_ARGS__); \
    common_Perf(aDebugString); \
  } while(0)

/**
  * @brief Macro to check result against expected value and report failure
  * @param[in] s String describing the operation
  * @param[in] result Result value to check
  * @param[in] expected_value Expected result value
  * @details Prints file and line number on mismatch, logs error code via UART, and exits,
  *          used for validation in the CN0575 project (e.g., ADIN1110 initialization).
  */
#define DEBUG_RESULT(s,result,expected_value) \
  do { \
    if ((result) != (expected_value)) { \
      sprintf(aDebugString,"%s  %d", __FILE__,__LINE__); \
      common_Fail(aDebugString); \
      sprintf(aDebugString, "%s Error Code: 0x%08" PRIX32 "\n\rFailed\n\r", (s), (uint32_t)(result)); \
      common_Perf(aDebugString); \
      exit(0); \
    } \
  } while (0)

/**
  * @brief Structure for SPI callback data
  * @details Holds callback function and user data for SPI interrupt handling, used with
  *          BSP_spi2_register_callback for ADIN1110 communication.
  */
typedef struct {
    ADI_CB   callback;    /*!< Callback function pointer. */
    void    *userData;    /*!< User-defined data passed to callback. */
} SPI_CallbackData;

/**
  * @brief SPI callback data instance
  * @details Global instance for storing SPI2 callback information for ADIN1110 events.
  */
extern SPI_CallbackData spiCallbackData;

/**
  * @brief Register an SPI callback function
  * @param [in] pfCallback Pointer to the callback function
  * @param [in] pCBParam User-defined parameter for the callback
  * @return 0 on success, non-zero on failure
  * @details Registers a callback for SPI2 events (e.g., transfer complete) with the ADIN1110.
  */
extern uint32_t HAL_SPI_Register_Callback(ADI_CB const *pfCallback, void *const pCBParam);

/**
  * @brief Select the ADIN1110 chip via CS pin
  * @details Sets the SPI1_CS pin low to enable SPI communication with the ADIN1110.
  */
void ADIN1110_CS_Select(void);

/**
  * @brief Deselect the ADIN1110 chip via CS pin
  * @details Sets the SPI1_CS pin high to disable SPI communication with the ADIN1110.
  */
void ADIN1110_CS_Deselect(void);

/*Functions prototypes*/

/**
  * @brief Initialize the STM32L496ZG-P system hardware (unused in CN0575)
  * @return 0 on success, non-zero on failure
  * @details Intended to configure system clocks and peripherals, but not called in the CN0575
  *          project; initialization is handled directly in main.c instead.
  */
uint32_t        BSP_InitSystem                  (void);

/**
  * @brief Get the current system tick count
  * @return Current tick count in milliseconds
  * @details Provides a FreeRTOS-compatible timestamp using TIM6 as the timebase source,
  *          used in standalone mode for heartbeat timing (250ms intervals) in the CN0575 project.
  */
uint32_t        BSP_SysNow                      (void);

/**
  * @brief Register an IRQ callback function
  * @param [in] intCallback Pointer to the IRQ callback function
  * @param [in] hDevice Device handle to pass to the callback
  * @return 0 on success, non-zero on failure
  * @details Registers a callback for ADIN1110 interrupt events (EXTI15_10_IRQn), invoked by
  *          HAL_GPIO_EXTI_Callback in main.c for INT_N pin handling in the CN0575 project.
  */
uint32_t        BSP_RegisterIRQCallback         (ADI_CB const *intCallback, void * hDevice);

/**
 * @brief Disable IRQ interrupts.
 * @details Disables interrupt requests for the ADIN1110 on the STM32L496ZG-P.
 */
void            BSP_DisableIRQ                  (void);

/**
 * @brief Enable IRQ interrupts.
 * @details Enables interrupt requests for the ADIN1110 on the STM32L496ZG-P.
 */
void            BSP_EnableIRQ                   (void);

/**
 * @brief Set the MDC pin state.
 * @param [in] set True to set high, false to set low.
 * @return 0 on success, non-zero on failure.
 * @details Controls the Management Data Clock pin (not used with ADIN1110 SPI).
 */
uint32_t        BSP_SetPinMDC                   (bool set);

/**
 * @brief Set the MDIO pin state.
 * @param [in] set True to set high, false to set low.
 * @return 0 on success, non-zero on failure.
 * @details Controls the Management Data I/O pin (not used with ADIN1110 SPI).
 */
uint32_t        BSP_SetPinMDIO                  (bool set);

/**
 * @brief Get the MDIO pin input state.
 * @return Pin state (0 or 1).
 * @details Reads the Management Data I/O pin (not used with ADIN1110 SPI).
 */
uint16_t        BSP_GetPinMDInput               (void);

/**
 * @brief Change the MDIO pin direction.
 * @param [in] output True for output, false for input.
 * @details Configures MDIO pin as input or output (not used with ADIN1110 SPI).
 */
void            BSP_ChangeMDIPinDir             (bool output);

/**
 * @brief Perform SPI0 write and read operation.
 * @param [in] pBufferTx Pointer to transmit buffer.
 * @param [out] pBufferRx Pointer to receive buffer.
 * @param [in] nbBytes Number of bytes to transfer.
 * @return 0 on success, non-zero on failure.
 * @details SPI0 transaction (not used for ADIN1110 in CN0575; likely SPI2 is used).
 */
uint32_t        BSP_spi0_write_and_read         (uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes);

/**
 * @brief Register SPI0 callback function.
 * @param [in] pfCallback Pointer to the callback function.
 * @param [in] pCBParam User-defined callback parameter.
 * @return 0 on success, non-zero on failure.
 * @details Registers a callback for SPI0 events (not used for ADIN1110 in CN0575).
 */
uint32_t        BSP_spi0_register_callback      (ADI_CB const *pfCallback, void *const pCBParam);

/**
  * @brief Perform SPI2 write and read operation for ADIN1110
  * @param [in] pBufferTx Pointer to transmit buffer
  * @param [out] pBufferRx Pointer to receive buffer
  * @param [in] nbBytes Number of bytes to transfer
  * @param [in] useDma True to use DMA, false for blocking transfer
  * @return 0 on success, non-zero on failure
  * @details Handles SPI1 communication with the ADIN1110 MAC-PHY (despite naming), supporting
  *          DMA or blocking mode for Ethernet frame transfers in the CN0575 project.
  */
uint32_t        BSP_spi2_write_and_read         (uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes, bool useDma);

/**
  * @brief Register SPI2 callback function for ADIN1110
  * @param [in] pfCallback Pointer to the callback function
  * @param [in] pCBParam User-defined callback parameter
  * @return 0 on success, non-zero on failure
  * @details Registers a callback for SPI1 events (despite naming), used for ADIN1110 transfer
  *          completion in both standalone and lwIP modes in the CN0575 project.
  */
uint32_t        BSP_spi2_register_callback      (ADI_CB const *pfCallback, void *const pCBParam);

/**
 * @brief Control the ADIN1110 hardware reset pin.
 * @param [in] set True to assert reset (low), false to release (high).
 * @details Toggles the reset pin for the ADIN1110 on the CN0575 SPE board.
 */
void            BSP_HWReset                     (bool set);

/**
 * @brief Toggle the heartbeat indicator.
 * @details Updates the heartbeat LED or other indicator to show system activity.
 */
void            BSP_HeartBeat                   (void);

/**
  * @brief Toggle the heartbeat indicator
  * @details Updates the heartbeat LED (LD3 on PB14) every 250ms in standalone mode or 1ms in
  *          lwIP mode via NetworkMaintenanceTask, showing system activity in the CN0575 project.
  */
void            BSP_HeartBeatLed                (bool on);

/**
 * @brief Set the error LED state.
 * @param [in] on True to turn on, false to turn off.
 * @details Controls the error LED to indicate system faults.
 */
void            BSP_ErrorLed                    (bool on);

/**
 * @brief Set the functional LED 1 state.
 * @param [in] on True to turn on, false to turn off.
 * @details Controls a user-defined functional LED (e.g., for status).
 */
void            BSP_FuncLed1                    (bool on);

/**
 * @brief Toggle functional LED 1.
 * @details Inverts the current state of functional LED 1.
 */
void            BSP_FuncLed1Toggle              (void);

//void            BSP_FuncLed2                    (bool on);
//void            BSP_FuncLed2Toggle              (void);

/**
 * @brief Toggle all LEDs.
 * @details Inverts the state of all configured LEDs on the board.
 */
void            BSP_LedToggleAll                (void);

/**
  * @brief Delay execution for a specified time
  * @param [in] delay Delay duration in milliseconds
  * @details Provides a blocking delay using FreeRTOS-compatible timing (via BSP_SysNow),
  *          used in lwIP mode for initial link setup (500ms) and standalone mode in the CN0575 project.
  */
void            BSP_delayMs                     (uint32_t delay);

/**
 * @brief Perform custom initialization.
 * @details Placeholder for project-specific initialization (e.g., ADXL345 setup).
 */
void            BSP_CustomInit                  (void);

/**
 * @brief Write a message to UART.
 * @param [in] ptr Pointer to the message string.
 * @return Number of bytes written.
 * @details Sends a debug or log message via UART.
 */
extern uint32_t msgWrite                        (char * ptr);

/**
 * @brief Submit a transmit buffer to UART.
 * @param [in] buffer Pointer to the transmit buffer.
 * @param [in] nbBytes Number of bytes to transmit.
 * @return HAL_OK on success, HAL error code otherwise.
 * @details Queues data for UART transmission (e.g., debug logs).
 */
extern HAL_StatusTypeDef submitTxBuffer         (uint8_t * buffer, int nbBytes);

#endif /* BOARDSUPPORT_H */

/** @} */
