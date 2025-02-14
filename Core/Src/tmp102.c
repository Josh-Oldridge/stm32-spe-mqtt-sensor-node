#include "tmp102.h"
#include "i2c.h"    // This file should declare hi2c2, the I2C2 handle.
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>

#define TMP102_ADDR        (0x48 << 1)  // 7-bit address 0x48 shifted left by 1 for HAL (change to 0x49 if ADD0 is high)
#define TMP102_TEMP_REG    0x00         // Temperature register address

float TMP102_ReadTemperature(void) {
	uint8_t reg = TMP102_TEMP_REG;
	uint8_t data[2] = { 0 };
	BaseType_t semStatus;

	if (i2cSemaphore == NULL) {
		I2C2_InitSemaphore();
		if (i2cSemaphore == NULL) {
			return -1000;
		}
	}

	if (HAL_I2C_Master_Transmit_DMA(&hi2c2, TMP102_ADDR, &reg, 1) != HAL_OK) {
		printf("TMP102: I2C DMA transmit error!\n");
		return -1000;
	}

	semStatus = xSemaphoreTake(i2cSemaphore, pdMS_TO_TICKS(100));
	if (semStatus != pdTRUE) {
		printf("TMP102: Timeout waiting for transmit complete!\n");
		return -1000;
	}

	if (HAL_I2C_Master_Receive_DMA(&hi2c2, TMP102_ADDR, data, 2) != HAL_OK) {
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
