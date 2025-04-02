/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    freertos.c
 * @brief   FreeRTOS Application Code for CN0575 Project
 * @details This file implements FreeRTOS tasks and initialization for the STM32L496ZG-P
 *          Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project. It manages
 *          network maintenance, sensor data collection (ADC, temperature, acceleration),
 *          and MQTT publishing for secure transmission of sensor data over TLSv1.2 via lwIP
 *          and the ADIN1110 MAC-PHY. Uses TIM6 as the timebase source for FreeRTOS scheduling.
 * @addtogroup freertos FreeRTOS Module
 * @{
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lwIP_adin1110_app.h"
#include "adin1110.h"
#include "ADIN1110_mac_addr_rdef.h"
#include "lwip/timeouts.h"
#include "lwip/dhcp.h"
#include "netif/etharp.h"
#include "lwip/udp.h"
#include "lwip/netif.h"
#include "tmp102.h"
#include "adxl345.h"
#include "i2c.h"
#include "adc.h"
#include "client_mqtt.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/sntp.h"
#include "semphr.h"
#include "sensor_data.h"
#include "lwipopts.h"
#include "lwip/ip_addr.h"
#include "rtc.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/** @brief Indicates if MQTT client is connected. */
extern volatile bool mqtt_connected;

/** @brief Indicates if MQTT client is attempting to connect. */
extern volatile bool mqtt_connecting;

/** @brief Tracks DHCP configuration status (0 = not configured). */
volatile int dhcp_configured = 0;

/** @brief Stores the latest sensor data readings. */
SensorData_t latestSensorData = { 0 };

/** @brief Mutex for synchronizing access to sensor data. */
SemaphoreHandle_t sensorDataMutex;

/** @brief Structure to hold averaged and maximum acceleration data from ADXL345. */
AveragedADXL345Data_t averagedADXL345Data = { 0 };

/** @brief Running sum of X-axis acceleration values for averaging. */
float ax_sum = 0.0f;
/** @brief Running sum of Y-axis acceleration values for averaging. */
float ay_sum = 0.0f;
/** @brief Running sum of Z-axis acceleration values for averaging. */
float az_sum = 0.0f;
/** @brief Maximum X-axis acceleration value observed during the sampling period. */
int16_t ax_max = -32768;
/** @brief Maximum Y-axis acceleration value observed during the sampling period. */
int16_t ay_max = -32768;
/** @brief Maximum Z-axis acceleration value observed during the sampling period. */
int16_t az_max = -32768;
/** @brief Number of acceleration samples collected in the current period. */
uint32_t sample_count = 0;

/** @brief Handle for the network maintenance task. */
osThreadId_t networkMaintenanceTaskHandle;

/** @brief Attributes for the network maintenance task. */
const osThreadAttr_t networkMaintenanceTask_attributes = { .name =
		"netMaintTask", .stack_size = 1024 * 8, .priority =
		(osPriority_t) osPriorityRealtime, };

/** @brief Handle for the ADC task. */
osThreadId_t adcTaskHandle;

/** @brief Attributes for the ADC task. */
const osThreadAttr_t adcTask_attributes = { .name = "adcTask",
		.stack_size = 512, .priority = (osPriority_t) osPriorityLow, };

/** @brief Handle for the temperature sensor task. */
osThreadId_t tempTaskHandle;

/** @brief Attributes for the temperature sensor task. */
const osThreadAttr_t tempTask_attributes = { .name = "tempTask", .stack_size =
		512 * 2, .priority = (osPriority_t) osPriorityLow, };

/** @brief Handle for the accelerometer task. */

/** @brief Attributes for the accelerometer task. */
osThreadId_t accelTaskHandle;
const osThreadAttr_t accelTask_attributes = { .name = "accelTask", .stack_size =
		512 * 2, .priority = (osPriority_t) osPriorityNormal, };

/** @brief Handle for the sensor data MQTT task. */
osThreadId_t sensorDataMQTTTaskHandle;

/** @brief Attributes for the sensor data MQTT task. */
const osThreadAttr_t sensorDataMQTTTask_attributes =
		{ .name = "sensorDataMQTTTask", .stack_size = 1024 * 8, .priority =
				osPriorityHigh, };

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
		.stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/**
 * @brief  Network maintenance task function
 * @param [in] argument  Task argument (not used)
 * @details Manages lwIP network stack, heartbeat LED, and link input for ADIN1110.
 */
void NetworkMaintenanceTask(void *argument);

/**
 * @brief  ADC task function
 * @param [in] argument  Task argument (not used)
 * @details Collects ADC data and updates shared sensor data structure.
 */
void ADCTask(void *argument);

/**
 * @brief  Temperature sensor task function
 * @param [in] argument  Task argument (not used)
 * @details Reads temperature from TMP102 and updates shared sensor data.
 */
void TempTask(void *argument);

/**
 * @brief  Accelerometer task function
 * @param [in] argument  Task argument (not used)
 * @details Reads acceleration data from ADXL345 and updates shared sensor data.
 */
void AccelTask(void *argument);

/**
 * @brief  Sensor data MQTT publishing task function
 * @param [in] argument  Task argument (not used)
 * @details Publishes sensor data via MQTT, manages connection retries.
 */
void SensorDataMQTTTask(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);
void vApplicationDaemonTaskStartupHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void) {
	/* Placeholder for configuring a timer for runtime stats; not implemented. */
}

__weak unsigned long getRunTimeCounterValue(void) {
	/* Placeholder for retrieving runtime counter value; returns 0 by default. */
	return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 2 */
/**
 * @brief  FreeRTOS idle hook
 * @details Called when no tasks are running; currently empty in the CN0575 project.
 */
void vApplicationIdleHook(void) {
}

/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
/**
 * @brief  FreeRTOS tick hook
 * @details Called on each FreeRTOS tick (driven by TIM6); currently empty in the CN0575 project.
 */
void vApplicationTickHook(void) {

}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */

/**
 * @brief  FreeRTOS stack overflow hook
 * @param [in] xTask  Handle of the task that overflowed
 * @param [in] pcTaskName  Name of the task that overflowed
 * @details Logs stack overflow and halts execution in the CN0575 project.
 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName) {
	printf("Stack overflow detected in task: %s\n", pcTaskName);
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */

/**
 * @brief  FreeRTOS memory allocation failure hook
 * @details Logs memory allocation failure and halts execution in the CN0575 project.
 */
void vApplicationMallocFailedHook(void) {
	printf("Memory allocation failed!\n");
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}
/* USER CODE END 5 */

/* USER CODE BEGIN DAEMON_TASK_STARTUP_HOOK */

/**
 * @brief  FreeRTOS daemon task startup hook
 * @details Called when daemon tasks start; currently empty in the CN0575 project.
 */
void vApplicationDaemonTaskStartupHook(void) {
}
/* USER CODE END DAEMON_TASK_STARTUP_HOOK */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/** @brief Creates mutex for sensor data access synchronization. */
	sensorDataMutex = xSemaphoreCreateMutex();
	if (sensorDataMutex == NULL) {
		printf("Failed to create sensor data mutex!\n");
		Error_Handler();
	}
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */

	/** @brief Creates tasks for network, ADC, temperature, acceleration, and MQTT operations. */
	networkMaintenanceTaskHandle = osThreadNew(NetworkMaintenanceTask, NULL,
			&networkMaintenanceTask_attributes);
	adcTaskHandle = osThreadNew(ADCTask, NULL, &adcTask_attributes);
	tempTaskHandle = osThreadNew(TempTask, NULL, &tempTask_attributes);
	accelTaskHandle = osThreadNew(AccelTask, NULL, &accelTask_attributes);
	sensorDataMQTTTaskHandle = osThreadNew(SensorDataMQTTTask, NULL,
			&sensorDataMQTTTask_attributes);
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread
 * @param [in] argument  Not used
 * @details Default task that runs an infinite loop with a minimal delay in the CN0575 project.
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument) {
	/* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
 * @brief  Network maintenance task
 * @param [in] argument  Not used
 * @details Periodically toggles the heartbeat LED, processes lwIP link input from the ADIN1110,
 *          and checks network timeouts every 1ms in the CN0575 project.
 */
void NetworkMaintenanceTask(void *argument) {
	const uint32_t heartbeatIntervalMs = 1;
	TickType_t lastWakeTime = xTaskGetTickCount();

	for (;;) {
		BSP_HeartBeat();
		while (pDataAvailable(&pQ[0])) {
			LwIP_ADIN1110LinkInput(&myConn.netif);
		}
		sys_check_timeouts();
		vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(heartbeatIntervalMs));
	}
}

/**
 * @brief  ADC data collection task
 * @param [in] argument  Not used
 * @details Reads ADC data every 9 seconds and updates the shared sensor data structure,
 *          using a mutex for synchronization in the CN0575 project.
 */
void ADCTask(void *argument) {
	for (;;) {
		uint16_t adc_value = adcBuffer[0];
		if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
			latestSensorData.adc_value = adc_value;
			xSemaphoreGive(sensorDataMutex);
		}
		osDelay(pdMS_TO_TICKS(5000));
	}
}

/**
 * @brief  Temperature sensor task
 * @param [in] argument  Not used
 * @details Reads temperature from TMP102 every 6 seconds, updates shared sensor data,
 *          and signals sensor readiness in the CN0575 project.
 */
void TempTask(void *argument) {
	for (;;) {
		float temp = TMP102_ReadTemperature();
		if (temp > -1000) {
			if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
				latestSensorData.temperature = temp;
				if (!sensors_ready && latestSensorData.accel_x != 0) {
					sensors_ready = true;
				}
				xSemaphoreGive(sensorDataMutex);
			}
		} else {
			printf("TMP102 Read Error\n");
		}
		osDelay(pdMS_TO_TICKS(5000));
	}
}

/**
 * @brief  Accelerometer data collection task
 * @param [in] argument  Not used
 * @details Waits for the system to be ready, initializes the ADXL345 accelerometer,
 *          and then reads acceleration data every 10ms. For each reading, it updates
 *          the running sums and maximum values for each axis, increments the sample count,
 *          and updates the latest sensor data structure. Uses a mutex for thread-safe
 *          access to shared data. Part of the CN0575 project.
 */
void AccelTask(void *argument) {
    int16_t ax, ay, az;
    HAL_StatusTypeDef ret;

    ret = ADXL345_Init(&hi2c1);
    if (ret != HAL_OK) {
        printf("ADXL345 Initialization Failed!\n");
    } else {
        printf("ADXL345 Initialized Successfully!\n");
    }

    for (;;) {
        ret = ADXL345_ReadAccel(&hi2c1, &ax, &ay, &az);
        if (ret == HAL_OK) {
            if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                ax_sum += ax;
                ay_sum += ay;
                az_sum += az;

                if (ax > ax_max) ax_max = ax;
                if (ay > ay_max) ay_max = ay;
                if (az > az_max) az_max = az;

                sample_count++;

                latestSensorData.accel_x = ax;
                latestSensorData.accel_y = ay;
                latestSensorData.accel_z = az;

                if (!sensors_ready && latestSensorData.temperature != 0.0f) {
                    sensors_ready = true;
                }

                xSemaphoreGive(sensorDataMutex);
            }
        } else {
            printf("ADXL345 Read Error!\n");
        }
        osDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief  Sensor data MQTT publishing task
 * @param [in] argument  Not used
 * @details Waits for DHCP configuration, initializes SNTP, and publishes sensor data
 *          via MQTT every 20 seconds in the CN0575 project. It calculates average
 *          acceleration values using accumulated sums and sample count, sets the
 *          maximum acceleration values, and resets the accumulators for the next
 *          interval. Handles MQTT connection and reconnection logic.
 */
void SensorDataMQTTTask(void *argument) {
    static TickType_t lastPublishTime = 0;
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(20000);
    ip_addr_t ntp_server_addr;

    while (!dhcp_supplied_address(&myConn.netif)) {
        printf("Sensor Data MQTT Task: Waiting for DHCP configuration...\n");
        osDelay(pdMS_TO_TICKS(2000));
    }

    if (ip4addr_aton("192.168.1.7", &ntp_server_addr) == 1) {
        sntp_setserver(0, &ntp_server_addr);
        sntp_init();
        printf("SNTP initialized.\n");
    } else {
        printf("Failed to parse NTP server IP address.\n");
    }
    osDelay(pdMS_TO_TICKS(2000));

    for (;;) {
        if (mqtt_connected) {
            TickType_t currentTime = xTaskGetTickCount();
            if (lastPublishTime != 0) {
#ifdef MQTT_CLIENT_DEBUG
                printf("Time since last publish: %lu ms\n",
                       (unsigned long)((currentTime - lastPublishTime) * portTICK_PERIOD_MS));
#endif
            }
            lastPublishTime = currentTime;

            if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                if (sample_count > 0) {
                    averagedADXL345Data.avg_accel_x = ax_sum / sample_count;
                    averagedADXL345Data.avg_accel_y = ay_sum / sample_count;
                    averagedADXL345Data.avg_accel_z = az_sum / sample_count;
                    averagedADXL345Data.max_accel_x = ax_max;
                    averagedADXL345Data.max_accel_y = ay_max;
                    averagedADXL345Data.max_accel_z = az_max;

                    ax_sum = 0.0f;
                    ay_sum = 0.0f;
                    az_sum = 0.0f;
                    ax_max = -32768;
                    ay_max = -32768;
                    az_max = -32768;
                    sample_count = 0;
                } else {
                    averagedADXL345Data.avg_accel_x = 0.0f;
                    averagedADXL345Data.avg_accel_y = 0.0f;
                    averagedADXL345Data.avg_accel_z = 0.0f;
                    averagedADXL345Data.max_accel_x = 0;
                    averagedADXL345Data.max_accel_y = 0;
                    averagedADXL345Data.max_accel_z = 0;
                }

#ifdef MQTT_CLIENT_DEBUG
                printf("ADXL345: Avg X=%.2f, Y=%.2f, Z=%.2f, Max X=%d, Y=%d, Z=%d\n",
                       averagedADXL345Data.avg_accel_x, averagedADXL345Data.avg_accel_y,
                       averagedADXL345Data.avg_accel_z, averagedADXL345Data.max_accel_x,
                       averagedADXL345Data.max_accel_y, averagedADXL345Data.max_accel_z);
#endif

                xSemaphoreGive(sensorDataMutex);
            }

            int ret = client_mqtt_publish_sensor_data();
            if (ret != ERR_OK) {
                printf("Publish failed: %d\n", ret);
                if (ret == ERR_CONN || ret == ERR_MEM) {
                    mqtt_connected = false;
                    mqtt_connecting = false;
                    printf("Connection lost, retrying in 5 seconds...\n");
                    osDelay(pdMS_TO_TICKS(5000));
                }
            }
        } else if (!mqtt_connecting) {
            printf("MQTT not connected, attempting to connect...\n");
            mqtt_connecting = true;
            client_mqtt_init();
            TickType_t startTime = xTaskGetTickCount();
            while (mqtt_connecting && !mqtt_connected
                   && (xTaskGetTickCount() - startTime < pdMS_TO_TICKS(10000))) {
                osDelay(pdMS_TO_TICKS(100));
            }
            if (mqtt_connecting && !mqtt_connected) {
                printf("Connection attempt timed out, retrying in 5 seconds...\n");
                mqtt_connecting = false;
                osDelay(pdMS_TO_TICKS(5000));
            }
        } else {
            printf("MQTT connection in progress, waiting...\n");
            osDelay(pdMS_TO_TICKS(1000));
        }
        vTaskDelayUntil(&lastWakeTime, frequency);
    }
}

/** @} */
/* USER CODE END Application */

