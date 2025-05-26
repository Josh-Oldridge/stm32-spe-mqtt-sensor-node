/**
 * @file    adxl345.c
 * @brief   Implementation of the ADXL345 Accelerometer Driver.
 * @details Implements I2C1 communication with the ADXL345 3-axis accelerometer on the
 *          STM32L496ZG-P Nucleo board for the CN0575 SPE board project. Collects XYZ
 *          acceleration data using DMA and FreeRTOS synchronization, which is later
 *          processed into MQTT packets for secure transmission via the ADIN1110 MAC-PHY.
 */

/** @addtogroup adxl345 ADXL345 Accelerometer Driver
 *  @{
 */

#include "adxl345.h"
#include "i2c.h"
#include "sensor_data.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>

/**
 * @brief I2C timeout in FreeRTOS ticks.
 * @details Defines a 1000ms timeout for I2C operations, converted to ticks using
 *          pdMS_TO_TICKS for FreeRTOS semaphore waits.
 */
#define I2C_TIMEOUT_TICKS   pdMS_TO_TICKS(1000)

/**
 * @brief Write a single register to the ADXL345 using DMA.
 * @param [in] hi2c1 Pointer to the I2C handle structure.
 * @param [in] reg Register address to write to.
 * @param [in] data 8-bit data value to write.
 * @return HAL_OK on success, HAL error code otherwise.
 * @details Uses DMA to transmit the register address and data over I2C, waiting for
 *          completion via a FreeRTOS semaphore with a timeout. Prints an error on timeout.
 */
static HAL_StatusTypeDef ADXL345_WriteRegister_DMA(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t data) {
    HAL_StatusTypeDef ret;
    uint8_t buffer[2];
    buffer[0] = reg;
    buffer[1] = data;

    ret = HAL_I2C_Master_Transmit_DMA(hi2c, ADXL345_I2C_ADDR, buffer, 2);
    if(ret != HAL_OK) {
        return ret;
    }
    if(xSemaphoreTake(i2cSemaphore, I2C_TIMEOUT_TICKS) != pdTRUE) {
        printf("ADXL345: Timeout waiting for write complete!\n");
        return HAL_TIMEOUT;
    }
    return HAL_OK;
}

/**
 * @brief Read one or more registers from the ADXL345 using DMA.
 * @param [in] hi2c Pointer to the I2C handle structure.
 * @param [in] reg Starting register address to read from.
 * @param [out] data Pointer to buffer for received data.
 * @param [in] size Number of bytes to read.
 * @return HAL_OK on success, HAL error code otherwise.
 * @details Performs a two-step DMA operation: transmits the register address, then receives
 *          the data, using FreeRTOS semaphores for synchronization with timeouts. Prints
 *          errors on timeout.
 */
static HAL_StatusTypeDef ADXL345_ReadRegister_DMA(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *data, uint16_t size) {
    HAL_StatusTypeDef ret;
    ret = HAL_I2C_Master_Transmit_DMA(hi2c, ADXL345_I2C_ADDR, &reg, 1);
    if(ret != HAL_OK) {
        return ret;
    }
    if(xSemaphoreTake(i2cSemaphore, I2C_TIMEOUT_TICKS) != pdTRUE) {
        printf("ADXL345: Timeout waiting for transmit complete before read!\n");
        return HAL_TIMEOUT;
    }
    ret = HAL_I2C_Master_Receive_DMA(hi2c, ADXL345_I2C_ADDR, data, size);
    if(ret != HAL_OK) {
        return ret;
    }
    if(xSemaphoreTake(i2cSemaphore, I2C_TIMEOUT_TICKS) != pdTRUE) {
        printf("ADXL345: Timeout waiting for read complete!\n");
        return HAL_TIMEOUT;
    }
    return HAL_OK;
}

/*
 * @brief Initialize the ADXL345 accelerometer.
 * @param [in] hi2c Pointer to the I2C handle structure (configured for STM32 I2C peripheral).
 * @return HAL_OK on success, HAL error code otherwise.
 * @details Configures the ADXL345 over I2C on the STM32L496ZG-P, setting it to measurement
 *          mode with a ±2g range and 10-bit resolution to collect XYZ acceleration data
 *          for the CN0575 SPE board. Verifies device ID before configuration. The data
 *          is later used in forming MQTT packets for transmission.
 */
HAL_StatusTypeDef ADXL345_Init(I2C_HandleTypeDef *hi2c) {
    HAL_StatusTypeDef ret;
    uint8_t devid;

    vTaskDelay(pdMS_TO_TICKS(1));

    ret = ADXL345_WriteRegister_DMA(hi2c, ADXL345_REG_POWER_CTL, 0x00);
    if(ret != HAL_OK) {
        printf("ADXL345: Failed to write POWER_CTL 0x00 (ret=%d)\n", ret);
        return ret;
    }

    ret = ADXL345_WriteRegister_DMA(hi2c, ADXL345_REG_POWER_CTL, 0x10);
    if(ret != HAL_OK) {
        printf("ADXL345: Failed to write POWER_CTL 0x10 (ret=%d)\n", ret);
        return ret;
    }

    ret = ADXL345_WriteRegister_DMA(hi2c, ADXL345_REG_POWER_CTL, 0x08);
    if(ret != HAL_OK) {
        printf("ADXL345: Failed to write POWER_CTL 0x08 (ret=%d)\n", ret);
        return ret;
    }

    ret = ADXL345_WriteRegister_DMA(hi2c, ADXL345_REG_DATA_FORMAT, 0x08);
    if(ret != HAL_OK) {
        printf("ADXL345: Failed to set data format (ret=%d)\n", ret);
        return ret;
    }

    ret = ADXL345_ReadRegister_DMA(hi2c, ADXL345_REG_DEVID, &devid, 1);
    if(ret != HAL_OK) {
        printf("ADXL345: Failed to read device ID (ret=%d)\n", ret);
        return ret;
    }

    printf("ADXL345: Read device ID: 0x%02X\n", devid);
    if(devid != 0xE5) {
        printf("ADXL345: Unexpected device ID: 0x%02X (expected 0xE5)\n", devid);
        return HAL_ERROR;
    }

    return HAL_OK;
}

/*
 * @brief Read acceleration data from the ADXL345.
 * @param [in] hi2c Pointer to the I2C handle structure.
 * @param [out] x Pointer to store the X-axis acceleration (signed 16-bit).
 * @param [out] y Pointer to store the Y-axis acceleration (signed 16-bit).
 * @param [out] z Pointer to store the Z-axis acceleration (signed 16-bit).
 * @return HAL_OK on success, HAL error code otherwise.
 * @details Reads 6 bytes of XYZ acceleration data from the ADXL345 via I2C, combining low
 *          and high bytes into signed 16-bit values for each axis. The data is collected
 *          for subsequent inclusion in MQTT packets in the CN0575 SPE board project.
 */
HAL_StatusTypeDef ADXL345_ReadAccel(I2C_HandleTypeDef *hi2c, int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buffer[6] = {0};
    HAL_StatusTypeDef ret;

    ret = ADXL345_ReadRegister_DMA(hi2c, ADXL345_REG_DATAX0, buffer, 6);
    if(ret != HAL_OK) {
        return ret;
    }

    *x = (int16_t)((buffer[1] << 8) | buffer[0]);
    *y = (int16_t)((buffer[3] << 8) | buffer[2]);
    *z = (int16_t)((buffer[5] << 8) | buffer[4]);

    return HAL_OK;
}

/** @} */
