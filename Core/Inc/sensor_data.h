/**
  ******************************************************************************
  * @file    sensor_data.h
  * @brief   Sensor Data Interface for CN0575 Project
  * @details This header defines the sensor data structure and synchronization primitives for
  *          the CN0575 Single Pair Ethernet (SPE) board project on the STM32L496ZG-P Nucleo
  *          board. Provides a shared data type (SensorData_t) for temperature (TMP102),
  *          acceleration (ADXL345), and ADC values (ADC1), accessed by freertos.c’s TempTask,
  *          AccelTask, and ADCTask, and published via MQTT in client_mqtt.c when USE_LWIP is
  *          defined. Uses a FreeRTOS mutex for thread-safe access.
  * @addtogroup sensor Sensor Data Management
  * @{
  ******************************************************************************
  */

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

/** @brief Include for FreeRTOS semaphore support */
#include "cmsis_os.h"

/** @brief Structure for holding sensor data
  * @details Contains latest readings from TMP102 (temperature), ADXL345 (acceleration), and
  *          ADC1 (analog value), shared across FreeRTOS tasks and MQTT publishing.
  */
typedef struct {
    float temperature;   /*!< Temperature in Celsius from TMP102 */
    int16_t accel_x;     /*!< X-axis acceleration from ADXL345 (raw units) */
    int16_t accel_y;     /*!< Y-axis acceleration from ADXL345 (raw units) */
    int16_t accel_z;     /*!< Z-axis acceleration from ADXL345 (raw units) */
    uint16_t adc_value;  /*!< ADC value from ADC1 (0-4095 range) */
} SensorData_t;

/** @brief Global instance of latest sensor readings
  * @details Updated by freertos.c tasks (TempTask, AccelTask, ADCTask) and read by
  *          client_mqtt.c for MQTT publishing.
  */
extern SensorData_t latestSensorData;

/** @brief Mutex for synchronizing access to latestSensorData
  * @details Protects sensor data from concurrent access by FreeRTOS tasks, initialized in
  *          freertos.c’s MX_FREERTOS_Init.
  */
extern SemaphoreHandle_t sensorDataMutex;

#endif /* SENSOR_DATA_H */

/**
  * @}
  */
