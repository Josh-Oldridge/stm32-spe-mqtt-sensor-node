#include "udp_query.h"
#include "lwip/udp.h"
#include "lwip/etharp.h"
#include "lwip/pbuf.h"
#include "lwip/timeouts.h"
#include "cmsis_os.h"
#include "main.h"
#include "lwIP_adin1110_app.h"
#include <string.h>

volatile QueryState_t queryState = STATE_IDLE;
volatile uint32_t querySentTime = 0;

static struct udp_pcb *query_udp_pcb = NULL;
static ip4_addr_t remoteIP;
#define REMOTE_UDP_PORT 5000
#define LOCAL_UDP_PORT  5001

static const char queryMsg[] = "CMD:QUERY:LD1_ON?";
static const char responseOnMsg[] = "CMD:RESPONSE:LD1_ON";
static const char responseOffMsg[] = "CMD:RESPONSE:LD1_OFF";

/**
 * @brief  Internal callback for UDP reception.
 * @note   This function is private to this file.
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
 * @brief  Sends a UDP query packet.
 * @retval lwIP error code.
 */
err_t udp_send_query(void) {
	struct pbuf *p;
	err_t err;

#ifdef TCP_IP_DEBUG
	if (myConn.netif.ip_addr.addr == 0) {
		DEBUG_MESSAGE(
				"[NETWORK] Skipping UDP query: No valid IP assigned yet.\r\n");
		return ERR_CONN;
	}
#endif /* TCP/IP_DEBUG */

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
 * @brief  Processes the UDP query state machine.*/

void process_udp_query(void) {
	static int queryRetryCount = 0;
	static uint32_t nextRetryTime = 0;
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
				DEBUG_MESSAGE(
						"[NETWORK] UDP Query timeout, retrying in %d ms... (%d/%d)\r\n",
						QUERY_TIMEOUT, queryRetryCount, MAX_QUERY_RETRIES);
				nextRetryTime = now + QUERY_TIMEOUT;
				queryState = STATE_WAITING_FOR_RETRY;
			} else {
				DEBUG_MESSAGE(
						"[NETWORK] Max UDP Query retries reached. Stopping retries.\r\n");
				queryState = STATE_IDLE;
			}
		}
		break;

	case STATE_WAITING_FOR_RETRY:
		if (now >= nextRetryTime) {
			DEBUG_MESSAGE("[NETWORK] Retrying UDP query...\r\n");
			udp_send_query();
			querySentTime = now;
			queryState = STATE_WAITING_FOR_RESPONSE;
		}
		break;

	case STATE_RESPONSE_RECEIVED:
		queryState = STATE_IDLE;
		queryRetryCount = 0;
		break;
	}
}
