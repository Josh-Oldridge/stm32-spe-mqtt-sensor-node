/**
  ******************************************************************************
  * @file    tmp102.c
  * @brief   TMP102 Temperature Sensor Driver for CN0575 Project
  * @details This file implements the temperature reading function for the TMP102 sensor on the
  *          STM32L496ZG-P Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project.
  *          Uses I2C1 with DMA to read the temperature register, synchronized via FreeRTOS
  *          semaphore, and updates latestSensorData in freertos.c’s TempTask every 6 seconds
  *          when USE_LWIP is defined. Logs errors to LPUART1 via printf.
  * @addtogroup sensor Sensor Drivers
  * @{
  ******************************************************************************
  */

/** @brief Includes for I2C, FreeRTOS, and logging
  * @details i2c.h for I2C1 access, FreeRTOS.h and semphr.h for semaphore, stdio.h for printf.
  */
#include "tmp102.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>

/** @brief I2C address of TMP102 (7-bit 0x48 shifted left by 1 for HAL)
  * @details Default address is 0x90 (0x48 << 1); change to 0x92 (0x49 << 1) if ADD0 pin is high.
  */
#define TMP102_ADDR        (0x48 << 1)

/** @brief Temperature register address in TMP102
  * @details Points to the read-only temperature register (0x00) for data retrieval.
  */
#define TMP102_TEMP_REG    0x00

/**
  * @brief Read temperature from TMP102 sensor
  * @return Temperature in Celsius, or -1000 on error
  * @details Reads the TMP102 temperature register via I2C2 with DMA, using a semaphore for
  *          synchronization. Converts 12-bit raw data to Celsius (0.0625°C/LSB), handling
  *          negative values with two’s complement. Returns -1000 on I2C or timeout errors,
  *          logging to LPUART1. Called by freertos.c’s TempTask every 6 seconds.
  */
float TMP102_ReadTemperature(void) {
	uint8_t reg = TMP102_TEMP_REG;
	uint8_t data[2] = { 0 };
	BaseType_t semStatus;

	if (i2cSemaphore == NULL) {
		I2C_InitSemaphore();
		if (i2cSemaphore == NULL) {
			return -1000;
		}
	}

	if (HAL_I2C_Master_Transmit_DMA(&hi2c1, TMP102_ADDR, &reg, 1) != HAL_OK) {
		printf("TMP102: I2C DMA transmit error!\n");
		return -1000;
	}

	semStatus = xSemaphoreTake(i2cSemaphore, pdMS_TO_TICKS(100));
	if (semStatus != pdTRUE) {
		printf("TMP102: Timeout waiting for transmit complete!\n");
		return -1000;
	}

	if (HAL_I2C_Master_Receive_DMA(&hi2c1, TMP102_ADDR, data, 2) != HAL_OK) {
		printf("TMP102: I2C DMA receive error!\n");
		return -1000;
	}

	semStatus = xSemaphoreTake(i2cSemaphore, pdMS_TO_TICKS(100));
	if (semStatus != pdTRUE) {
		printf("TMP102: Timeout waiting for receive complete!\n");
		return -1000;
	}

	uint16_t rawTemp = (data[0] << 8) | data[1];
	rawTemp >>= 4;

    if (rawTemp & 0x800) {
        rawTemp = (~rawTemp + 1) & 0xFFF;
        return -((float)rawTemp * 0.0625f);
    } else {
        return (float)rawTemp * 0.0625f;
    }
}


/**
 * @}
 */
