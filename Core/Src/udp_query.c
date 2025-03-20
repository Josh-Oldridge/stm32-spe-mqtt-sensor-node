/**
  ******************************************************************************
  * @file    udp_query.c
  * @brief   UDP Query Implementation for CN0575 Project (Unused)
  * @details This file implements a UDP query mechanism for the CN0575 Single Pair Ethernet (SPE)
  *          board project on the STM32L496ZG-P Nucleo board. Originally used to send UDP packets
  *          to 192.168.1.11:5000 every second, toggling LD1 based on responses ("LD1_ON"/"LD1_OFF"),
  *          ensuring ICMP ping responses by keeping ADIN1110 buffers active. Currently unused,
  *          replaced by interrupt-driven ADIN1110 handling in freertos.c when USE_LWIP is defined.
  *          Logs debug messages via LPUART1 using boardsupport.h’s DEBUG_MESSAGE.
  * @addtogroup network Network Utilities
  * @{
  ******************************************************************************
  */

#include "udp_query.h"
#include "lwip/udp.h"
#include "lwip/etharp.h"
#include "lwip/pbuf.h"
#include "lwip/timeouts.h"
#include "cmsis_os.h"
#include "main.h"
#include "lwIP_adin1110_app.h"
#include <string.h>

/** @brief Current state of the UDP query process
  * @details Tracks the state machine state, volatile as it’s updated by callbacks and tasks.
  */
volatile QueryState_t queryState = STATE_IDLE;

/** @brief Timestamp of the last UDP query sent
  * @details Records the send time in milliseconds, volatile for access across contexts.
  */
volatile uint32_t querySentTime = 0;

/** @brief UDP protocol control block for queries
  * @details Static PCB for sending/receiving UDP packets, initialized in udp_send_query.
  */
static struct udp_pcb *query_udp_pcb = NULL;

/** @brief Remote IP address for UDP queries
  * @details Set to 192.168.1.11, the test laptop IP used during development.
  */
static ip4_addr_t remoteIP;

/** @brief Remote UDP port for sending queries
  * @details Set to 5000, the port on 192.168.1.11 for receiving test queries.
  */
#define REMOTE_UDP_PORT 5000

/** @brief Local UDP port for binding
  * @details Set to 5001, the port on CN0575 for sending/receiving test queries.
  */
#define LOCAL_UDP_PORT  5001

/** @brief Query message sent via UDP
  * @details String "CMD:QUERY:LD1_ON?" sent to test LD1 control on 192.168.1.11.
  */
static const char queryMsg[] = "CMD:QUERY:LD1_ON?";

/** @brief Response message indicating LD1 on
  * @details String "CMD:RESPONSE:LD1_ON" expected from 192.168.1.11 to set LD1 high.
  */
static const char responseOnMsg[] = "CMD:RESPONSE:LD1_ON";

/** @brief Response message indicating LD1 off
  * @details String "CMD:RESPONSE:LD1_OFF" expected from 192.168.1.11 to set LD1 low.
  */
static const char responseOffMsg[] = "CMD:RESPONSE:LD1_OFF";

/**
  * @brief Callback for receiving UDP responses
  * @param [in] arg User argument (unused)
  * @param [in] pcb UDP protocol control block
  * @param [in] p Received pbuf with UDP data
  * @param [in] addr Sender’s IP address
  * @param [in] port Sender’s port
  * @details Processes UDP responses from 192.168.1.11:5000, toggling LD1 based on "LD1_ON"/"LD1_OFF"
  *          messages, logging via DEBUG_MESSAGE. Updates queryState to STATE_RESPONSE_RECEIVED.
  */
static void udp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
		const ip_addr_t *addr, u16_t port) {
	if (p == NULL) {
		DEBUG_MESSAGE("Received NULL pbuf in UDP callback\r\n");
		return;
	}
	char recv_buf[128] = { 0 };
	size_t copy_len =
			(p->len < sizeof(recv_buf) - 1) ? p->len : sizeof(recv_buf) - 1;
	memcpy(recv_buf, p->payload, copy_len);
	recv_buf[copy_len] = '\0';
	DEBUG_MESSAGE("UDP Received: %s\r\n", recv_buf);

	if (strncmp(recv_buf, responseOnMsg, strlen(responseOnMsg)) == 0) {
		queryState = STATE_RESPONSE_RECEIVED;
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
		DEBUG_MESSAGE("LD1 turned ON via UDP response\r\n");
	} else if (strncmp(recv_buf, responseOffMsg, strlen(responseOffMsg)) == 0) {
		queryState = STATE_RESPONSE_RECEIVED;
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
		DEBUG_MESSAGE("LD1 turned OFF via UDP response\r\n");
	} else {
		DEBUG_MESSAGE("Unknown UDP message received: %s\r\n", recv_buf);
	}
	pbuf_free(p);
}

/**
  * @brief Send a UDP query packet
  * @return ERR_OK on success, lwIP error code on failure
  * @details Sends "CMD:QUERY:LD1_ON?" to 192.168.1.11:5000 via UDP, initializing PCB and binding to
  *          LOCAL_UDP_PORT if needed. Sets queryState and logs via DEBUG_MESSAGE. Returns error on
  *          allocation or send failure.
  */
err_t udp_send_query(void) {
    struct pbuf *p;
    err_t err;

#ifdef UDP_QUERY_DEBUG
    if (myConn.netif.ip_addr.addr == 0) {
        DEBUG_MESSAGE("[NETWORK] Skipping UDP query: No valid IP assigned yet.\r\n");
        return ERR_CONN;
    }
#endif

    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(queryMsg) - 1, PBUF_RAM);
    if (p == NULL) {
        DEBUG_MESSAGE("[ERROR] Failed to allocate pbuf for UDP query\r\n");
        return ERR_MEM;
    }
    memcpy(p->payload, queryMsg, sizeof(queryMsg) - 1);

    if (query_udp_pcb == NULL) {
        query_udp_pcb = udp_new();
        if (query_udp_pcb == NULL) {
            DEBUG_MESSAGE("[ERROR] Failed to create UDP PCB\r\n");
            pbuf_free(p);
            return ERR_MEM;
        }
        err = udp_bind(query_udp_pcb, IP4_ADDR_ANY, LOCAL_UDP_PORT);
        if (err != ERR_OK) {
            DEBUG_MESSAGE("[ERROR] UDP bind failed: %d\r\n", err);
            udp_remove(query_udp_pcb);  // Clean up PCB on failure
            query_udp_pcb = NULL;
            pbuf_free(p);
            return err;
        }
        udp_recv(query_udp_pcb, udp_recv_callback, NULL);
    }
    IP4_ADDR(&remoteIP, 192, 168, 1, 11);
    err = udp_sendto(query_udp_pcb, p, &remoteIP, REMOTE_UDP_PORT);
    if (err == ERR_OK) {
        querySentTime = BSP_SysNow();
        queryState = STATE_WAITING_FOR_RESPONSE;
        DEBUG_MESSAGE("[NETWORK] UDP Query sent at %lu ms\r\n", querySentTime);
    } else {
        DEBUG_MESSAGE("[ERROR] UDP send failed: %d\r\n", err);
    }
    pbuf_free(p);
    return err;
}

/**
  * @brief Process the UDP query state machine
  * @details Manages UDP query retries (up to MAX_QUERY_RETRIES) with QUERY_TIMEOUT, toggling between
  *          idle, waiting, and response states. Logs progress via DEBUG_MESSAGE. Originally called
  *          every second for testing, now unused in favor of ADIN1110 interrupts in freertos.c.
  */
void process_udp_query(void) {
    static int queryRetryCount = 0;
    uint32_t now = BSP_SysNow();

    switch (queryState) {
    case STATE_IDLE:
        DEBUG_MESSAGE("[NETWORK] Initiating new UDP query...\r\n");
        udp_send_query();
        queryRetryCount = 0;
        querySentTime = now;
        queryState = STATE_WAITING_FOR_RESPONSE;
        break;

    case STATE_WAITING_FOR_RESPONSE:
        if ((now - querySentTime) >= QUERY_TIMEOUT) {
            if (queryRetryCount < MAX_QUERY_RETRIES) {
                queryRetryCount++;
                DEBUG_MESSAGE("[NETWORK] UDP Query timeout, retrying... (%d/%d)\r\n",
                              queryRetryCount, MAX_QUERY_RETRIES);
                udp_send_query();  // Retry immediately
                querySentTime = now;
                queryState = STATE_WAITING_FOR_RESPONSE;
            } else {
                DEBUG_MESSAGE("[NETWORK] Max UDP Query retries reached. Stopping.\r\n");
                queryState = STATE_IDLE;
            }
        }
        break;

    case STATE_RESPONSE_RECEIVED:
        queryState = STATE_IDLE;
        queryRetryCount = 0;
        break;

    default:
        break;
    }
}

/**
  * @}
  */
