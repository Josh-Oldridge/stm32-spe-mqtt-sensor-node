#include "adxl345.h"
#include <stdio.h>

// Helper: Write one byte to a register.
static HAL_StatusTypeDef ADXL345_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t data) {
    return HAL_I2C_Mem_Write(hi2c, ADXL345_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

// Helper: Read multiple bytes starting from a register.
static HAL_StatusTypeDef ADXL345_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *data, uint16_t size) {
    return HAL_I2C_Mem_Read(hi2c, ADXL345_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, size, 100);
}

// Initialize the ADXL345 sensor.
HAL_StatusTypeDef ADXL345_Init(I2C_HandleTypeDef *hi2c) {
    HAL_StatusTypeDef ret;
    uint8_t devid;

    // Optionally wait a short time for sensor power-up
    HAL_Delay(10);

    // "Wake up" the sensor by writing to POWER_CTL
    ret = ADXL345_WriteRegister(hi2c, ADXL345_REG_POWER_CTL, 0x00);
    if (ret != HAL_OK) {
        printf("ADXL345: Failed to write POWER_CTL 0x00 (ret=%d)\n", ret);
        return ret;
    }
    HAL_Delay(10);

    ret = ADXL345_WriteRegister(hi2c, ADXL345_REG_POWER_CTL, 0x10);
    if (ret != HAL_OK) {
        printf("ADXL345: Failed to write POWER_CTL 0x10 (ret=%d)\n", ret);
        return ret;
    }
    HAL_Delay(10);

    ret = ADXL345_WriteRegister(hi2c, ADXL345_REG_POWER_CTL, 0x08);
    if (ret != HAL_OK) {
        printf("ADXL345: Failed to write POWER_CTL 0x08 (ret=%d)\n", ret);
        return ret;
    }
    HAL_Delay(10);

    // Set DATA_FORMAT: full resolution mode, ±2g range.
    ret = ADXL345_WriteRegister(hi2c, ADXL345_REG_DATA_FORMAT, 0x08);
    if (ret != HAL_OK) {
        printf("ADXL345: Failed to set data format (ret=%d)\n", ret);
        return ret;
    }

    // Now read the device ID register.
    ret = ADXL345_ReadRegister(hi2c, ADXL345_REG_DEVID, &devid, 1);
    if (ret != HAL_OK) {
        printf("ADXL345: Failed to read device ID (ret=%d)\n", ret);
        return ret;
    }
    printf("ADXL345: Read device ID: 0x%02X\n", devid);
    if (devid != 0xE5) {
        printf("ADXL345: Unexpected device ID: 0x%02X (expected 0xE5)\n", devid);
        return HAL_ERROR;
    }

    return HAL_OK;
}

// Read acceleration data (X, Y, Z).
HAL_StatusTypeDef ADXL345_ReadAccel(I2C_HandleTypeDef *hi2c, int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buffer[6] = {0};
    HAL_StatusTypeDef ret = ADXL345_ReadRegister(hi2c, ADXL345_REG_DATAX0, buffer, 6);
    if (ret != HAL_OK) {
        return ret;
    }

    // Combine bytes into 16-bit two's complement values.
    *x = (int16_t)((buffer[1] << 8) | buffer[0]);
    *y = (int16_t)((buffer[3] << 8) | buffer[2]);
    *z = (int16_t)((buffer[5] << 8) | buffer[4]);

    return HAL_OK;
}
