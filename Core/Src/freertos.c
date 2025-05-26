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
#ifdef STATS
#include "lwip/ip_addr.h"
#include "lwip/inet_chksum.h"
#include "adin1110_enhanced_stats.h"
#include "lwip/icmp.h"
#include "lwip/raw.h"
#include <math.h>
#endif /* STATS */

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

extern LwIP_ADIN1110_t myConn;

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

#ifdef STATS
/** @brief Handle for the statistics task. */
osThreadId_t pingTaskHandle;

/** @brief Attributes for the statistics task. */
const osThreadAttr_t pingTask_attributes = {
    .name = "pingTask",
    .stack_size = 32768,
    .priority = osPriorityNormal,
};
#endif /* STATS */

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
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/**
 * @brief  Network maintenance task function
 * @param [in] argument  Task argument (not used)
 * @details Manages lwIP network stack, heartbeat LED, and link input for ADIN1110.
 */
void NetworkMaintenanceTask(void *argument);


#ifdef STATS

/**
 * @brief  Ping Task Stats function
 * @param [in] argument  Task argument (not used)
 * @details Sends 10000 pings requests, collects metrics every 10 seconds and averages them, and prints out the statistics when finished
 */
void PingTask(void *argument);

static u8_t ping_recv_callback(void *arg, struct raw_pcb *pcb, struct pbuf *p, const struct ip4_addr *addr);
static uint32_t ping_sent = 0;
static uint32_t ping_received = 0;
static uint16_t ping_seq_num = 0;
static uint32_t ping_lost = 0;
const uint32_t maxPings = 10000; // Send 5000 pings

/**
 * @brief Callback for receiving ICMP echo replies
 * @param [in] arg User argument (unused)
 * @param [in] pcb Raw protocol control block
 * @param [in] p Received packet buffer
 * @param [in] addr Source IP address (IPv4)
 * @return 1 if packet processed, 0 otherwise
 */
static u8_t ping_recv_callback(void *arg, struct raw_pcb *pcb, struct pbuf *p, const struct ip4_addr *addr) {
    if (p != NULL && p->len >= (20 + sizeof(struct icmp_echo_hdr))) {
        struct icmp_echo_hdr *icmp = (struct icmp_echo_hdr *)((uint8_t *)p->payload + 20);
        uint16_t seq = lwip_ntohs(icmp->seqno);
        u8_t icmp_type = ICMPH_TYPE(icmp);
        printf("Received packet from %s, len=%u, type=%u, code=%u, id=0x%04X, seq=%u\n",
               ipaddr_ntoa(addr), p->tot_len, icmp_type, ICMPH_CODE(icmp), lwip_ntohs(icmp->id), seq);
        printf("\n");
        if (icmp_type == 0 && seq >= 1 && seq <= maxPings) {
        	taskENTER_CRITICAL();
            ping_received++;
            printf("PingTask: Received ping reply #%lu\n", ping_received);
            fflush(stdout);
            taskEXIT_CRITICAL();
        } else if (icmp_type == 11) {
            printf("PingTask: Received Time Exceeded (type 11), code %u\n", ICMPH_CODE(icmp));
        } else {
            printf("PingTask: Unexpected ICMP type %u, expected Echo Reply (0)\n", icmp_type);
        }
        pbuf_free(p);
        return 1;
    } else {
        printf("PingTask: Packet too short, len=%u\n", p->tot_len);
        pbuf_free(p);
    }
    return 0;
}

#endif /* STATS */

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
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */

	/** @brief Creates tasks for network, stats, ADC, temperature, acceleration, and MQTT operations. */
	networkMaintenanceTaskHandle = osThreadNew(NetworkMaintenanceTask, NULL,
			&networkMaintenanceTask_attributes);

#ifdef STATS
	pingTaskHandle = osThreadNew(PingTask, myConn.hDevice, &pingTask_attributes);

#else
	adcTaskHandle = osThreadNew(ADCTask, NULL, &adcTask_attributes);
	tempTaskHandle = osThreadNew(TempTask, NULL, &tempTask_attributes);
	accelTaskHandle = osThreadNew(AccelTask, NULL, &accelTask_attributes);
	sensorDataMQTTTaskHandle = osThreadNew(SensorDataMQTTTask, NULL,
			&sensorDataMQTTTask_attributes);
#endif /* STATS*/
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

#ifdef STATS


/**
 * @brief Ping task for network testing and statistics collection
 * @param [in] argument Pointer to the ADIN1110 device handle
 * @details Performs a ping test by sending 10000 ICMP echo requests to 192.168.1.7,
 *          collects link quality metrics from the ADIN1110 every 10 pings, and prints
 *          statistics after the test completes.
 */
void PingTask(void *argument) {
	adin1110_DeviceHandle_t *hDevice = (adin1110_DeviceHandle_t*) argument;
	const TickType_t pingFrequency = pdMS_TO_TICKS(5);
	const TickType_t perPingTimeout = pdMS_TO_TICKS(5);
	const TickType_t timeout = pdMS_TO_TICKS(60000);
	const uint16_t payload_size = 1472;
	TickType_t xLastPingTime = xTaskGetTickCount();
	TickType_t testStartTime;
	LinkQualitySample samples[NUM_SAMPLES] = { 0 };
	uint32_t sampleIndex = 0;
	adi_eth_Result_e result;
	uint16_t regVal;

	printf("PingTask: Started with hDevice=%p, *hDevice=%p\n", hDevice,
			hDevice ? *hDevice : NULL);
	fflush(stdout);
	if (hDevice == NULL || *hDevice == NULL) {
		printf("PingTask: Invalid device handle\n");
		fflush(stdout);
		vTaskDelete(NULL);
	}

	printf("PingTask: Waiting for DHCP configuration...\n");
	while (!dhcp_supplied_address(&myConn.netif)) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	printf("PingTask: DHCP complete, IP assigned\n");

	struct raw_pcb *pcb = raw_new(IP_PROTO_ICMP);
	if (pcb == NULL) {
		printf("PingTask: Failed to create raw PCB\n");
		fflush(stdout);
		vTaskDelete(NULL);
	}
	raw_bind(pcb, IP_ADDR_ANY);
	raw_recv(pcb, ping_recv_callback, NULL);

	ip_addr_t target;
	ipaddr_aton("192.168.1.7", &target);

	ping_sent = 0;
	ping_received = 0;
	ping_lost = 0;
	ping_seq_num = 0;
	testStartTime = xTaskGetTickCount();

	printf("PingTask: Checking diagnostics clock (0x1E882C)\n");
	result = adin1110_PhyRead(*hDevice, ADDR_CRSM_DIAG_CLK_CTRL, &regVal);
	if (result == ADI_ETH_SUCCESS) {
		printf("Debug: Diagnostics Clock (0x1E882C): %s\n",
				(regVal & BITM_CRSM_DIAG_CLK_CTRL_CRSM_DIAG_CLK_EN) ?
						"Enabled" : "Disabled");
		if (!(regVal & BITM_CRSM_DIAG_CLK_CTRL_CRSM_DIAG_CLK_EN)) {
			printf("Debug: Enabling diagnostics clock (0x1E882C)\n");
			result = adin1110_PhyWrite(*hDevice, ADDR_CRSM_DIAG_CLK_CTRL,
					BITM_CRSM_DIAG_CLK_CTRL_CRSM_DIAG_CLK_EN);
			if (result != ADI_ETH_SUCCESS) {
				printf(
						"Error: Failed to enable diagnostics clock (0x1E882C): 0x%08X\n",
						result);
			}
		}
	} else {
		printf("Error: Failed to read diagnostics clock (0x1E882C): 0x%08X\n",
				result);
	}

	printf("PingTask: Starting test with 10000 pings to %s\n",
			ipaddr_ntoa(&target));
	while (ping_sent < maxPings) {
		if (xTaskGetTickCount() - xLastPingTime >= pingFrequency) {
			struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT,
					sizeof(struct icmp_echo_hdr) + payload_size, PBUF_RAM);
			if (p != NULL) {
				struct icmp_echo_hdr *icmp = (struct icmp_echo_hdr*) p->payload;
				ICMPH_TYPE_SET(icmp, ICMP_ECHO);
				ICMPH_CODE_SET(icmp, 0);
				icmp->chksum = 0;
				icmp->id = 0x1234;
				icmp->seqno = lwip_htons(++ping_seq_num);

				uint8_t *payload = (uint8_t*) (icmp + 1);
				for (int i = 0; i < payload_size; i++) {
					payload[i] = 0x55;
				}

				icmp->chksum = inet_chksum(icmp,
						sizeof(struct icmp_echo_hdr) + payload_size);

				err_t err = raw_sendto(pcb, p, &target);
				if (err == ERR_OK) {
					ping_sent++;
					printf("PingTask: Sent ping #%lu to %s\n", ping_sent,
							ipaddr_ntoa(&target));

					TickType_t pingStartTime = xTaskGetTickCount();
					uint32_t expectedReceived = ping_received + 1;
					while (ping_received < expectedReceived
							&& (xTaskGetTickCount() - pingStartTime)
									< perPingTimeout) {
						vTaskDelay(pdMS_TO_TICKS(1));
					}
					if (ping_received < expectedReceived) {
						ping_lost++;
						printf("PingTask: Timeout waiting for reply #%lu\n",
								ping_sent);
					}

					if (ping_sent % 10 == 0 && sampleIndex < NUM_SAMPLES) {
						printf("PingTask: Collecting sample %lu at ping %lu\n",
								sampleIndex + 1, ping_sent);
						result = collectLinkQualityStats(hDevice,
								&samples[sampleIndex]);
						if (result != ADI_ETH_SUCCESS) {
							printf(
									"Error: Failed to collect sample %lu: 0x%08X\n",
									sampleIndex + 1, result);
						}
						sampleIndex++;
					}

				} else {
					printf("PingTask: Failed to send ping #%lu: %d\n",
							ping_sent + 1, err);
				}
				pbuf_free(p);
			}
			xLastPingTime = xTaskGetTickCount();
		}
		vTaskDelay(1);
	}

	printf("PingTask: Waiting for replies (timeout in %lu ms)\n",
			timeout * portTICK_PERIOD_MS);
	while (ping_received < ping_sent
			&& (xTaskGetTickCount() - testStartTime) < timeout) {
		vTaskDelay(10);
	}

	uint32_t ping_lost = ping_sent - ping_received;
	float packet_loss =
			ping_sent > 0 ? (float) ping_lost / ping_sent * 100.0f : 0.0f;
	printf("PingTask: Ping test complete\n");
	printf("Ping Stats:\n");
	printf("  Sent: %lu\n", ping_sent);
	printf("  Received: %lu\n", ping_received);
	printf("  Lost: %lu\n", ping_lost);
	printf("  Packet Loss: %.2f%%\n", packet_loss);
	printf("PingTask: Calling printEnhancedStats for ping...\n");
	printEnhancedStats(hDevice, samples);
	printf("PingTask: Test finished, deleting task\n");
	vTaskDelete(NULL);
}
#endif /* STATS */


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

