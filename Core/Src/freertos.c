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
osThreadId_t networkTaskHandle;
const osThreadAttr_t networkTask_attributes = { .name = "networkTask",
		.stack_size = 512 * 4,  // Increase stack if needed
		.priority = (osPriority_t) osPriorityHigh, };
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
		.stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };

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
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

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
void NetworkTask(void *argument) {
	uint32_t lastPollTime = 0;
	uint32_t lastArpPrint = 0;
	uint32_t last_arp_time = 0;
	uint32_t heartbeatCheckTime = 0;
	uint32_t pollIntervalMs = 500;
	static uint8_t link_was_up = 0;

	DEBUG_MESSAGE("[NETWORK] Waiting for a valid IP address...\n");

	while (myConn.netif.ip_addr.addr == 0) {
		osDelay(DHCP_CHECK_INTERVAL_MS);
		sys_check_timeouts();

		struct pbuf *p;
		while ((p = (struct pbuf*) readPQ(&pQ[0])) != NULL) {
			uint8_t *payload = (uint8_t*) p->payload;
			uint16_t ethType = (payload[12] << 8) | payload[13];

			if (ethType == 0x0806) {
				DEBUG_MESSAGE("[ARP] ARP Packet received before DHCP\n");
				netif_input(p, &myConn.netif);
			} else if (ethType == 0x0800) {
				uint8_t ipProtocol = payload[23];

				if (ipProtocol == 17) {
					uint16_t udpSrcPort = (payload[34] << 8) | payload[35];
					uint16_t udpDstPort = (payload[36] << 8) | payload[37];

					DEBUG_MESSAGE("UDP Packet - Src Port: %d, Dst Port: %d\n",
							udpSrcPort, udpDstPort);

					if (udpDstPort == 67 || udpDstPort == 68) {
						DEBUG_MESSAGE("[DHCP] DHCP Packet received!\n");
					}

					netif_input(p, &myConn.netif);
				} else {
					DEBUG_MESSAGE(
							"[NETWORK] Ignoring non-DHCP IPv4 packet...\n");
					pbuf_free(p);
				}
			} else {
				DEBUG_MESSAGE("[NETWORK] Unknown EtherType: 0x%04X\n", ethType);
				pbuf_free(p);
			}
		}

		if (myConn.netif.ip_addr.addr
				!= 0&& myConn.netif.ip_addr.addr != IPADDR_ANY) {
			DEBUG_MESSAGE("[NETWORK] IP Assigned: %s\n",
					ip4addr_ntoa(&myConn.netif.ip_addr));
			break;
		}
		DEBUG_MESSAGE("[NETWORK] Still waiting for IP assignment...\n");
	}
	if (myConn.netif.ip_addr.addr != 0
			&& dhcp_supplied_address(&myConn.netif)) {
		DEBUG_MESSAGE("[NETWORK] Successfully obtained IP: %d.%d.%d.%d\n",
				ip4_addr1(&myConn.netif.ip_addr),
				ip4_addr2(&myConn.netif.ip_addr),
				ip4_addr3(&myConn.netif.ip_addr),
				ip4_addr4(&myConn.netif.ip_addr));

		netif_set_up(&myConn.netif);
		netif_set_default(&myConn.netif);
		etharp_gratuitous(&myConn.netif);
	}

	while (1) {
		uint32_t now = BSP_SysNow();

		if ((now - lastPollTime) >= pollIntervalMs) {
			lastPollTime = now;
			sys_check_timeouts();
			LwIP_ADIN1110LinkInput(&myConn.netif);
			if (myConn.netif.ip_addr.addr != 0
					&& dhcp_supplied_address(&myConn.netif)) {
				process_udp_query();
			} else {
				DEBUG_MESSAGE(
						"[NETWORK] Skipping UDP queries, IP not assigned yet.\r\n");
			}
		}

		if ((now - heartbeatCheckTime) >= 250) {
			heartbeatCheckTime = now;
			BSP_HeartBeat();
		}

		if ((now - last_arp_time) >= 500) {
			last_arp_time = now;
			etharp_tmr();
		}

		uint8_t current_link_status = netif_is_link_up(&myConn.netif);
		if (current_link_status != link_was_up) {
			link_was_up = current_link_status;
			DEBUG_MESSAGE(link_was_up ? "Link is UP\r\n" : "Link is DOWN\r\n");

			if (!link_was_up) {
				DEBUG_MESSAGE(
						"[NETWORK] Link Down - Attempting DHCP Renewal...\n");
				osDelay(3000);

				if (!dhcp_supplied_address(&myConn.netif)) {
					DEBUG_MESSAGE(
							"[NETWORK] DHCP Renewal failed, restarting DHCP...\n");
					dhcp_release(&myConn.netif);
					etharp_cleanup_netif(&myConn.netif);
					dhcp_start(&myConn.netif);

					while (myConn.netif.ip_addr.addr == 0) {
						osDelay(DHCP_CHECK_INTERVAL_MS);
						sys_check_timeouts();
						DEBUG_MESSAGE("[NETWORK] Waiting for new IP...\n");
					}
				}

				DEBUG_MESSAGE("[NETWORK] New IP assigned: %d.%d.%d.%d\n",
						ip4_addr1(&myConn.netif.ip_addr),
						ip4_addr2(&myConn.netif.ip_addr),
						ip4_addr3(&myConn.netif.ip_addr),
						ip4_addr4(&myConn.netif.ip_addr));
			}
		}

		osDelay(1);
	}
}

/* USER CODE END Application */

