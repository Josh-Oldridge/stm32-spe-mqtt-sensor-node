/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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
#include "udp_query.h"
#include "lwip/udp.h"
#include "lwip/netif.h"
#include "tmp102.h"
#include "adxl345.h"
#include "i2c.h"
#include "adc.h"
#include "client_mqtt.h"
#include "lwip/apps/mqtt.h"
#include "semphr.h"
#include "sensor_data.h"
#include "lwipopts.h"

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
extern ADC_HandleTypeDef hadc1;
extern volatile bool mqtt_connected;
extern volatile bool mqtt_connecting;
volatile int dhcp_configured = 0;
volatile bool system_ready = false;
SensorData_t latestSensorData = {0};
SemaphoreHandle_t sensorDataMutex;


osThreadId_t networkMaintenanceTaskHandle;
const osThreadAttr_t networkMaintenanceTask_attributes = {
    .name = "netMaintTask",
    .stack_size = 1024 * 8,
    .priority = (osPriority_t) osPriorityRealtime,
};

osThreadId_t adcTaskHandle;
const osThreadAttr_t adcTask_attributes = {
    .name = "adcTask",
    .stack_size = 512,
    .priority = (osPriority_t) osPriorityLow,
};

osThreadId_t tempTaskHandle;
const osThreadAttr_t tempTask_attributes = {
    .name = "tempTask",
    .stack_size = 512 * 2,
    .priority = (osPriority_t) osPriorityLow,
};

osThreadId_t accelTaskHandle;
const osThreadAttr_t accelTask_attributes = {
    .name = "accelTask",
    .stack_size = 512 * 2,
    .priority = (osPriority_t) osPriorityLow,
};

osThreadId_t sensorDataMQTTTaskHandle;
const osThreadAttr_t sensorDataMQTTTask_attributes = {
    .name = "sensorDataMQTTTask",
    .stack_size = 1024 * 8,
    .priority = osPriorityHigh,
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void NetworkMaintenanceTask(void *argument);
void ADCTask(void *argument);
void TempTask(void *argument);
void AccelTask(void *argument);
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
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 2 */
void vApplicationIdleHook(void) {
}

/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
void vApplicationTickHook(void) {

}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
	printf("Stack overflow detected in task: %s\n", pcTaskName);
	    taskDISABLE_INTERRUPTS();
	    for (;;);
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
    printf("Memory allocation failed!\n");
    taskDISABLE_INTERRUPTS();
    for (;;);
}
/* USER CODE END 5 */

/* USER CODE BEGIN DAEMON_TASK_STARTUP_HOOK */
void vApplicationDaemonTaskStartupHook(void)
{
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
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  networkMaintenanceTaskHandle = osThreadNew(NetworkMaintenanceTask, NULL, &networkMaintenanceTask_attributes);
  adcTaskHandle = osThreadNew(ADCTask, NULL, &adcTask_attributes);
  tempTaskHandle = osThreadNew(TempTask, NULL, &tempTask_attributes);
  accelTaskHandle = osThreadNew(AccelTask, NULL, &accelTask_attributes);
  sensorDataMQTTTaskHandle = osThreadNew(SensorDataMQTTTask, NULL, &sensorDataMQTTTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
    /* Infinite loop */
	for (;;) {
			osDelay(1);
	}
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
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

void ADCTask(void *argument) {
    for (;;) {
        uint16_t adc_value = adcBuffer[0];
        if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            latestSensorData.adc_value = adc_value;
            xSemaphoreGive(sensorDataMutex);
        }
        osDelay(pdMS_TO_TICKS(9000));
    }
}

void TempTask(void *argument) {
	while (!system_ready) {
		osDelay(pdMS_TO_TICKS(100));
	}
	for (;;) {
		float temp = TMP102_ReadTemperature();
		if (temp > -1000) {
			if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
				latestSensorData.temperature = temp;
				xSemaphoreGive(sensorDataMutex);
			}
		} else {
			printf("TMP102 Read Error\n");
		}
		osDelay(pdMS_TO_TICKS(6000));
	}
}

void AccelTask(void *argument) {

	while (!system_ready) {
		osDelay(pdMS_TO_TICKS(100));
	}
	int16_t ax, ay, az;
	HAL_StatusTypeDef ret;

	ret = ADXL345_Init(&hi2c2);
	if (ret != HAL_OK) {
		printf("ADXL345 Initialization Failed!\n");
	} else {
		printf("ADXL345 Initialized Successfully!\n");
	}

	for (;;) {
		ret = ADXL345_ReadAccel(&hi2c2, &ax, &ay, &az);
		if (ret == HAL_OK) {
			if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
				latestSensorData.accel_x = ax;
				latestSensorData.accel_y = ay;
				latestSensorData.accel_z = az;
				xSemaphoreGive(sensorDataMutex);
			}
		} else {
			printf("ADXL345 Read Error!\n");
		}
		osDelay(pdMS_TO_TICKS(3000));
	}
}

void SensorDataMQTTTask(void *argument) {
	static TickType_t lastPublishTime = 0;
	TickType_t lastWakeTime = xTaskGetTickCount();
	const TickType_t frequency = pdMS_TO_TICKS(20000);

	while (!dhcp_supplied_address(&myConn.netif)) {
		printf("Sensor Data MQTT Task: Waiting for DHCP configuration...\n");
		osDelay(pdMS_TO_TICKS(2000));
	}
	for (;;) {
		if (mqtt_connected) {
			system_ready = true;
			TickType_t currentTime = xTaskGetTickCount();
			if (lastPublishTime != 0) {
				printf("Time since last publish: %lu ms\n",
				       (unsigned long)((currentTime - lastPublishTime) * portTICK_PERIOD_MS));
			}
			lastPublishTime = currentTime;

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
            while (mqtt_connecting && !mqtt_connected &&
                   (xTaskGetTickCount() - startTime < pdMS_TO_TICKS(10000))) {
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
/* USER CODE END Application */

