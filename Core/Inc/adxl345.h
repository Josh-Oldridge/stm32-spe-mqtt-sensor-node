/**
 * @file    adxl345.h
 * @brief   Definitions for the ADXL345 Accelerometer Driver.
 * @details Provides register definitions and function prototypes for interfacing with
 *          the ADXL345 3-axis accelerometer over I2C on the STM32L496ZG-P Nucleo board,
 *          part of the CN0575 SPE board project. Collects XYZ acceleration data, which is
 *          later processed and included in MQTT packets for secure transmission over
 *          TLSv1.2 via the ADIN1110 MAC-PHY, using lwIP and mbedtls. Assumes SDO pin tied
 *          to GND for I2C address selection.
 */

/** @addtogroup adxl345 ADXL345 Accelerometer Driver
 *  @{
 */

#ifndef ADXL345_H
#define ADXL345_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"

/**
 * @brief ADXL345 I2C Address (7-bit, shifted for HAL).
 * @details Fixed 7-bit address (0x53) when SDO is tied to GND, left-shifted by 1 for
 *          compatibility with STM32 HAL I2C functions (8-bit format).
 */
#define ADXL345_I2C_ADDR    (0x53 << 1)

/**
 * @brief ADXL345 Device ID Register.
 * @details Read-only register containing the device ID (0xE5 for ADXL345).
 */
#define ADXL345_REG_DEVID        0x00

/**
 * @brief ADXL345 Power Control Register.
 * @details Configures power modes (e.g., measurement enable) and sleep settings.
 */
#define ADXL345_REG_POWER_CTL    0x2D

/**
 * @brief ADXL345 Data Format Register.
 * @details Sets data resolution, range (e.g., ±2g, ±4g), and justification.
 */
#define ADXL345_REG_DATA_FORMAT  0x31

/**
 * @brief ADXL345 X-Axis Data Low Byte Register.
 * @details Lower 8 bits of the 16-bit X-axis acceleration data.
 */
#define ADXL345_REG_DATAX0       0x32

/**
 * @brief ADXL345 X-Axis Data High Byte Register.
 * @details Upper 8 bits of the 16-bit X-axis acceleration data.
 */
#define ADXL345_REG_DATAX1       0x33

/**
 * @brief ADXL345 Y-Axis Data Low Byte Register.
 * @details Lower 8 bits of the 16-bit Y-axis acceleration data.
 */
#define ADXL345_REG_DATAY0       0x34

/**
 * @brief ADXL345 Y-Axis Data High Byte Register.
 * @details Upper 8 bits of the 16-bit Y-axis acceleration data.
 */
#define ADXL345_REG_DATAY1       0x35

/**
 * @brief ADXL345 Z-Axis Data Low Byte Register.
 * @details Lower 8 bits of the 16-bit Z-axis acceleration data.
 */
#define ADXL345_REG_DATAZ0       0x36

/**
 * @brief ADXL345 Z-Axis Data High Byte Register.
 * @details Upper 8 bits of the 16-bit Z-axis acceleration data.
 */
#define ADXL345_REG_DATAZ1       0x37

/**
 * @brief Initialize the ADXL345 accelerometer.
 * @param [in] hi2c Pointer to the I2C handle structure (configured for STM32 I2C peripheral).
 * @return HAL_OK on success, HAL error code otherwise.
 * @details Configures the ADXL345 over I2C on the STM32L496ZG-P, setting it to measurement
 *          mode with a ±2g range and 10-bit resolution to collect XYZ acceleration data
 *          for the CN0575 SPE board. Verifies device ID before configuration. The data
 *          is later used in forming MQTT packets for transmission.
 */
HAL_StatusTypeDef ADXL345_Init(I2C_HandleTypeDef *hi2c);

/**
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
HAL_StatusTypeDef ADXL345_ReadAccel(I2C_HandleTypeDef *hi2c, int16_t *x, int16_t *y, int16_t *z);

#ifdef __cplusplus
}
#endif

#endif /* ADXL345_H */

/** @} */
