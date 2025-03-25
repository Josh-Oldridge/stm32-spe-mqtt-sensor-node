/**
  ******************************************************************************
  * @file    tmp102.h
  * @brief   TMP102 Temperature Sensor Interface for CN0575 Project
  * @details This header declares the temperature reading function for the TMP102 sensor on the
  *          STM32L496ZG-P Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project.
  *          Provides temperature data via I2C1, used by freertos.c’s TempTask to update
  *          latestSensorData for MQTT publishing when USE_LWIP is defined.
  * @addtogroup sensor Sensor Drivers
  * @{
  ******************************************************************************
  */

#ifndef TMP102_H
#define TMP102_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief Read temperature from TMP102 sensor
  * @return Temperature in Celsius, or -1000 on error
  * @details Reads the temperature register via I2C1 with DMA, returning the value in degrees
  *          Celsius or an error code (-1000) if I2C communication fails, used in freertos.c.
  */
float TMP102_ReadTemperature(void);

#ifdef __cplusplus
}
#endif

#endif /* TMP102_H */

/**
  * @}
  */
