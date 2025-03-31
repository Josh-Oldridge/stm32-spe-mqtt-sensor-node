/**
  ******************************************************************************
  * @file    sensor_data.h
  * @brief   Sensor Data Interface for CN0575 Project
  * @details This header defines data structures and synchronization primitives for
  *          managing raw and processed sensor data in the CN0575 Single Pair Ethernet
  *          (SPE) board project on the STM32L496ZG-P Nucleo board. It includes structures
  *          for the latest raw readings from temperature (TMP102), acceleration (ADXL345),
  *          and ADC (ADC1) sensors, as well as processed acceleration data (averages and
  *          maximums). The data is accessed by tasks in freertos.c and published via MQTT
  *          in client_mqtt.c when USE_LWIP is defined. A FreeRTOS mutex ensures thread-safe
  *          access to shared data.
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

/** @brief Structure for holding processed acceleration data
 * @details Contains the average and maximum acceleration values for each axis,
 *          calculated over a sampling period and used for MQTT publishing.
 */
typedef struct {
    float avg_accel_x;   /*!< Average X-axis acceleration */
    float avg_accel_y;   /*!< Average Y-axis acceleration */
    float avg_accel_z;   /*!< Average Z-axis acceleration */
    int16_t max_accel_x; /*!< Maximum X-axis acceleration */
    int16_t max_accel_y; /*!< Maximum Y-axis acceleration */
    int16_t max_accel_z; /*!< Maximum Z-axis acceleration */
} AveragedADXL345Data_t;

/** @brief Global instance of latest sensor readings
  * @details Updated by freertos.c tasks (TempTask, AccelTask, ADCTask) and read by
  *          client_mqtt.c for MQTT publishing.
  */
extern SensorData_t latestSensorData;

/** @brief Global instance of processed acceleration data
 * @details Updated periodically by SensorDataMQTTTask in freertos.c and used for MQTT publishing.
 */
extern AveragedADXL345Data_t averagedADXL345Data;

/** @brief Mutex for synchronizing access to latestSensorData
  * @details Protects sensor data from concurrent access by FreeRTOS tasks, initialized in
  *          freertos.c’s MX_FREERTOS_Init.
  */
extern SemaphoreHandle_t sensorDataMutex;

/** @brief Running sum of X-axis acceleration values for averaging */
extern float ax_sum;

/** @brief Running sum of Y-axis acceleration values for averaging */
extern float ay_sum;

/** @brief Running sum of Z-axis acceleration values for averaging */
extern float az_sum;

/** @brief Maximum X-axis acceleration value observed during the sampling period */
extern int16_t ax_max;

/** @brief Maximum Y-axis acceleration value observed during the sampling period */
extern int16_t ay_max;

/** @brief Maximum Z-axis acceleration value observed during the sampling period */
extern int16_t az_max;

/** @brief Number of acceleration samples collected in the current sampling period */
extern uint32_t sample_count;

#endif /* SENSOR_DATA_H */

/**
  * @}
  */
