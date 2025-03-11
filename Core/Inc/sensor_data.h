#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include "cmsis_os.h"

typedef struct {
    float temperature;
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    uint16_t adc_value;
} SensorData_t;

extern SensorData_t latestSensorData;
extern SemaphoreHandle_t sensorDataMutex;

#endif /* SENSOR_DATA_H */
