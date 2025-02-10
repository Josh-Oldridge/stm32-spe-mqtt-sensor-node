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
#include "lwip/timeouts.h"
#include "lwip/dhcp.h"
#include "netif/etharp.h"
#include "udp_query.h"
#include "net_listen.h"
#include "lwip/udp.h"
#include "lwip/netif.h"
#include "tmp102.h"
#include "adxl345.h"
#include "i2c.h"
#include "client_mqtt.h"
#include "net_events.h"
#include "lwip/apps/mqtt.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define HEARTBEAT_INTERVAL 5000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern uint16_t adcBuffer[ADC_BUFFER_SIZE];
extern ADC_HandleTypeDef hadc1;

volatile int dhcp_configured = 0;
//osThreadId_t networkTaskHandle;
//const osThreadAttr_t networkTask_attributes = { .name = "networkTask",
//		.stack_size = 512 * 4,  // Increase stack if needed
//		.priority = (osPriority_t) osPriorityHigh, };
osThreadId_t networkMaintenanceTaskHandle;
const osThreadAttr_t networkMaintenanceTask_attributes = {
    .name = "netMaintTask",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t) osPriorityHigh,
};


osSemaphoreId_t adcSemaphoreHandle;

osThreadId_t adcTaskHandle;
const osThreadAttr_t adcTask_attributes = {
    .name = "adcTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t tempTaskHandle;
const osThreadAttr_t tempTask_attributes = {
    .name = "tempTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t accelTaskHandle;
const osThreadAttr_t accelTask_attributes = {
    .name = "accelTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t mqttTaskHandle;
const osThreadAttr_t mqttTask_attributes = {
    .name = "mqttTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityNormal,
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
//void NetworkTask(void *argument);
void NetworkMaintenanceTask(void *argument);
void ADCTask(void *argument);
void TempTask(void *argument);
void AccelTask(void *argument);
void MQTTTask(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);
void vApplicationDaemonTaskStartupHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
void vApplicationTickHook( void )
{
   /* This function will be called by each tick interrupt if
   configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
   added here, but the tick hook is called from an interrupt context, so
   code must not attempt to block, and only the interrupt safe FreeRTOS API
   functions can be used (those that end in FromISR()). */
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
	/* add mutexes, ... */
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
//  networkTaskHandle = osThreadNew(NetworkTask, NULL, &networkTask_attributes);
  networkMaintenanceTaskHandle = osThreadNew(NetworkMaintenanceTask, NULL, &networkMaintenanceTask_attributes);
//  adcTaskHandle = osThreadNew(ADCTask, NULL, &adcTask_attributes);
//  tempTaskHandle = osThreadNew(TempTask, NULL, &tempTask_attributes);
//  accelTaskHandle = osThreadNew(AccelTask, NULL, &accelTask_attributes);
  mqttTaskHandle = osThreadNew(MQTTTask, NULL, &mqttTask_attributes);
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
//void NetworkTask(void *argument) {
//    uint32_t heartbeatCheckTime = 0;
//    const uint32_t heartbeatIntervalMs = 100;
//    uint32_t arpLastTime = 0;
//    const uint32_t arpInterval = 30000;
//
//    static uint32_t lastHeartbeatTime = 0;
//    uint32_t now;
//    err_t err;
//    bool announced = false;
//
//
//    err = net_listen_init();
//    if (err != ERR_OK) {
//        DEBUG_MESSAGE("[NETWORK] ICMP listener initialization failed: %d\r\n", err);
//    }
//
//    err = heartbeat_udp_init();
//    if (err != ERR_OK) {
//        DEBUG_MESSAGE("[HEARTBEAT] Failed to initialize heartbeat UDP PCB: %d\r\n", err);
//    }
//
//	while (1) {
//		now = BSP_SysNow();
//
//		if ((now - heartbeatCheckTime) >= heartbeatIntervalMs) {
//			heartbeatCheckTime = now;
//			BSP_HeartBeat();
//			sys_check_timeouts();
//		}
//
//		if (dhcp_supplied_address(&myConn.netif)) {
//			if (!announced) {
//				etharp_gratuitous(&myConn.netif);
//				arpLastTime = now;
//				announced = true;
//			}
//
//			if ((now - arpLastTime) >= arpInterval) {
//				arpLastTime = now;
//				etharp_gratuitous(&myConn.netif);
//			}
//		}
//
//		LwIP_ADIN1110LinkInput(&myConn.netif);
//
//		if (dhcp_supplied_address(&myConn.netif)) {
//
//			net_listen_process();
//
//			if ((now - lastHeartbeatTime) >= HEARTBEAT_INTERVAL) {
//				send_heartbeat();
//				lastHeartbeatTime = now;
//			}
//		}
//
//
//		osDelay(1);
//    }
//}

void NetworkMaintenanceTask(void *argument) {
    uint32_t heartbeatCheckTime = 0;
    const uint32_t heartbeatIntervalMs = 100;
    uint32_t arpLastTime = 0;
    const uint32_t arpInterval = 30000;
    uint32_t now;
    bool announced = false;


    for (;;) {
        now = BSP_SysNow();

        if ((now - heartbeatCheckTime) >= heartbeatIntervalMs) {
            heartbeatCheckTime = now;
            BSP_HeartBeat();
            sys_check_timeouts();
        }

        if (dhcp_supplied_address(&myConn.netif)) {

        	xEventGroupSetBits(netEventGroup, NET_EVENT_DHCP_CONFIGURED);


//            if (!announced) {
//                etharp_gratuitous(&myConn.netif);
//                arpLastTime = now;
//                announced = true;
//            }
//            if ((now - arpLastTime) >= arpInterval) {
//                arpLastTime = now;
//                etharp_gratuitous(&myConn.netif);
//            }
        }

        LwIP_ADIN1110LinkInput(&myConn.netif);
        osDelay(1);
    }
}

//void ADCTask(void *argument) {
//    // Start ADC conversion once in continuous mode.
//    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcBuffer, ADC_BUFFER_SIZE) != HAL_OK) {
//        printf("ADC DMA Start Failed!\n");
//        // Optionally handle error here.
//    }
//    for (;;) {
//        // Wait 10 seconds between samples.
//        osDelay(pdMS_TO_TICKS(10000));
//
//        // Read the most recent ADC conversion value.
//        uint16_t adcValue = adcBuffer[0];
//        float voltage = (adcValue * 3.3f) / 4095.0f;
//        float current = (voltage - 1.65f) / 0.185f;
//        printf("ADC Value: %d, Voltage: %.2f V, Current: %.2f A\n",
//               adcValue, voltage, current);
//    }
//}
//
//void TempTask(void *argument) {
//    for (;;) {
//        float temp = TMP102_ReadTemperature();
//        if (temp > -1000) {  // Check for error value
//            printf("TMP102 Temperature: %.2f °C\n", temp);
//        } else {
//            printf("TMP102 Read Error\n");
//        }
//        osDelay(pdMS_TO_TICKS(15000));  // Wait 10 seconds before the next reading
//    }
//}
//
//void AccelTask(void *argument) {
//    int16_t ax, ay, az;
//    HAL_StatusTypeDef ret;
//
//    // Initialize the ADXL345 sensor using I2C2.
//    ret = ADXL345_Init(&hi2c2);
//    if (ret != HAL_OK) {
//        printf("ADXL345 Initialization Failed!\n");
//    } else {
//        printf("ADXL345 Initialized Successfully!\n");
//    }
//
//    for (;;) {
//        ret = ADXL345_ReadAccel(&hi2c2, &ax, &ay, &az);
//        if (ret == HAL_OK) {
//            printf("Accel: X=%d, Y=%d, Z=%d\n", ax, ay, az);
//        } else {
//            printf("ADXL345 Read Error!\n");
//        }
//        osDelay(pdMS_TO_TICKS(20000));  // Poll every 12 seconds
//    }
//}

void MQTTTask(void *argument) {
    // Wait until DHCP is configured (with a timeout if desired)
    EventBits_t uxBits;
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(5000); // wait up to 5 seconds
    uxBits = xEventGroupWaitBits(netEventGroup, NET_EVENT_DHCP_CONFIGURED, pdFALSE, pdTRUE, xMaxBlockTime);
    if ((uxBits & NET_EVENT_DHCP_CONFIGURED) == 0) {
        printf("DHCP not configured within timeout. Proceeding anyway...\n");
    } else {
        printf("DHCP configured, proceeding with MQTT initialization.\n");
        // Additional delay to allow lwIP routing/ARP tables to settle
        osDelay(pdMS_TO_TICKS(500));  // 500 ms extra delay
    }

    // Initialize MQTT client.
    client_mqtt_init();
    osDelay(pdMS_TO_TICKS(2000));  // Allow time for connection establishment

    for (;;) {
        // Check if we are still connected.
        if (!mqtt_client_is_connected(mqtt_client)) {
            printf("MQTT connection lost, reconnecting...\n");
            client_mqtt_init();
            osDelay(pdMS_TO_TICKS(2000)); // Give time for reconnection
        } else {
            // Publish sensor data if connected.
            client_mqtt_publish_sensor_data();
        }
        osDelay(pdMS_TO_TICKS(10000)); // Publish every 10 seconds
    }
}




/* USER CODE END Application */

