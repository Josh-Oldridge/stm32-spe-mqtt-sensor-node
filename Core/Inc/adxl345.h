#ifndef ADXL345_H
#define ADXL345_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"  // Adjust include as needed

// ADXL345 I2C Address (7-bit) - Assuming SD0 is tied to GND.
// (Shifted left by 1 for the HAL functions)
#define ADXL345_I2C_ADDR    (0x53 << 1)

// ADXL345 Register Addresses
#define ADXL345_REG_DEVID        0x00
#define ADXL345_REG_POWER_CTL    0x2D
#define ADXL345_REG_DATA_FORMAT  0x31
#define ADXL345_REG_DATAX0       0x32
#define ADXL345_REG_DATAX1       0x33
#define ADXL345_REG_DATAY0       0x34
#define ADXL345_REG_DATAY1       0x35
#define ADXL345_REG_DATAZ0       0x36
#define ADXL345_REG_DATAZ1       0x37

// Function prototypes
HAL_StatusTypeDef ADXL345_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef ADXL345_ReadAccel(I2C_HandleTypeDef *hi2c, int16_t *x, int16_t *y, int16_t *z);

#ifdef __cplusplus
}
#endif

#endif /* ADXL345_H */
