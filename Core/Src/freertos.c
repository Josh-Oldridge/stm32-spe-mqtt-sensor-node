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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define HEARTBEAT_INTERVAL 1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t networkTaskHandle;
const osThreadAttr_t networkTask_attributes = { .name = "networkTask",
		.stack_size = 512 * 4,  // Increase stack if needed
		.priority = (osPriority_t) osPriorityHigh, };
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
void NetworkTask(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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
	/* add semaphores, ... */
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
	networkTaskHandle = osThreadNew(NetworkTask, NULL, &networkTask_attributes);
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
void NetworkTask(void *argument) {
    uint32_t heartbeatCheckTime = 0;
    const uint32_t heartbeatIntervalMs = 100;
    uint32_t arpLastTime = 0;
    const uint32_t arpInterval = 30000;

    static uint32_t lastHeartbeatTime = 0;
    uint32_t now;
    err_t err;
    bool announced = false;


    // Initialize the ICMP listener (if used)
    err = net_listen_init();
    if (err != ERR_OK) {
        DEBUG_MESSAGE("[NETWORK] ICMP listener initialization failed: %d\r\n", err);
    }


//    // Initialize the heartbeat UDP PCB
    err = heartbeat_udp_init();
    if (err != ERR_OK) {
        DEBUG_MESSAGE("[HEARTBEAT] Failed to initialize heartbeat UDP PCB: %d\r\n", err);
    }

	while (1) {
		now = BSP_SysNow();

		if ((now - heartbeatCheckTime) >= heartbeatIntervalMs) {
			heartbeatCheckTime = now;
			BSP_HeartBeat();
			sys_check_timeouts();
		}

		if (dhcp_supplied_address(&myConn.netif)) {
			// If we haven't yet sent a gratuitous ARP after obtaining a valid IP,
			// do it now and initialize arpLastTime.
			if (!announced) {
				etharp_gratuitous(&myConn.netif);
				arpLastTime = now;
				announced = true;
			}

			// Optionally, periodically send gratuitous ARP every 60 seconds
			if ((now - arpLastTime) >= arpInterval) {
				arpLastTime = now;
				etharp_gratuitous(&myConn.netif);
			}
		}

		LwIP_ADIN1110LinkInput(&myConn.netif);

		if (dhcp_supplied_address(&myConn.netif)) {

			net_listen_process();
		}

        // Send a heartbeat every HEARTBEAT_INTERVAL (15 seconds)
        if ((now - lastHeartbeatTime) >= HEARTBEAT_INTERVAL) {
            send_heartbeat();
            lastHeartbeatTime = now;
        }
        osDelay(1);
    }
}

/* USER CODE END Application */

