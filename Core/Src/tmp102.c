#include "tmp102.h"
#include "i2c.h"    // This file should declare hi2c2, the I2C2 handle.
#include <stdio.h>

#define TMP102_ADDR        (0x48 << 1)  // 7-bit address 0x48 shifted left by 1 for HAL (change to 0x49 if ADD0 is high)
#define TMP102_TEMP_REG    0x00         // Temperature register address

float TMP102_ReadTemperature(void) {
    uint8_t reg = TMP102_TEMP_REG;
    uint8_t data[2] = {0};

    // Transmit the register address (0x00) to the TMP102
    if (HAL_I2C_Master_Transmit(&hi2c2, TMP102_ADDR, &reg, 1, 100) != HAL_OK) {
        printf("TMP102: I2C transmit error!\n");
        return -1000;  // error value
    }

    // Receive 2 bytes from the temperature register
    if (HAL_I2C_Master_Receive(&hi2c2, TMP102_ADDR, data, 2, 100) != HAL_OK) {
        printf("TMP102: I2C receive error!\n");
        return -1000;  // error value
    }

    // Combine the two bytes into a 16-bit raw value
    uint16_t rawTemp = (data[0] << 8) | data[1];

    // In 12-bit mode, the data is left-justified, so shift right 4 bits.
    rawTemp >>= 4;

    // Check if the temperature is negative (TMP102 uses two's complement for negative values)
    if (rawTemp & 0x800) {  // if sign bit (bit 11) is set
        rawTemp = (~rawTemp + 1) & 0xFFF;  // convert from two's complement (12-bit)
        return -((float)rawTemp * 0.0625f);
    } else {
        return (float)rawTemp * 0.0625f;
    }
}
