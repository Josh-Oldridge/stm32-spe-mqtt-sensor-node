/*
 *---------------------------------------------------------------------------
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.
 * and its licensors.By using this software you agree to the terms of the
 * associated Analog Devices Software License Agreement.
 *
 *---------------------------------------------------------------------------
 */
#ifdef USE_LWIP

#include "lwIP_adin1110_app.h"
#include "netif/etharp.h"
#include "lwip/ip_addr.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "adi_mac.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"

#define ADIN1110_INIT_ITER  (5)
#define MAX_FRAME_BUF_SIZE  (MAX_FRAME_SIZE + 4 + 2 + 4)
#define NUM_RX_DESC  4 /* 4 RX Descriptors for Frames sent from Host */
#define NUM_TX_DESC  4 /* 4 TX Descriptors for Frames sent by ADIN1110 */
#define ETHERNET_MTU        (1500)
#define IFNAME0         'e'
#define IFNAME1         '0'
#define HOSTNAME         "ADI_10BASE-T1L_Demo"
#define NETIF_LINK_SPEED_IN_BPS 10000000

HAL_ALIGNED_PRAGMA(4)static uint8_t rxBuf[NUM_RX_DESC][MAX_FRAME_BUF_SIZE] HAL_ALIGNED_ATTRIBUTE(4);
static adi_eth_BufDesc_t rxBufDesc[NUM_RX_DESC];

HAL_ALIGNED_PRAGMA(4)static uint8_t txBuf[NUM_TX_DESC][MAX_FRAME_BUF_SIZE] HAL_ALIGNED_ATTRIBUTE(4);
static adi_eth_BufDesc_t txBufDesc[NUM_TX_DESC];
static bool txBufAvailable[NUM_TX_DESC];
static int txBufIndex = 0;

HAL_ALIGNED_PRAGMA(4)pQueue_t pQ[MAX_PQ] HAL_ALIGNED_ATTRIBUTE(4);

static void initPQueue(pQueue_t *pQ);
static void* readPQ(pQueue_t *pQ);
static void writePQ(pQueue_t *pQ, uint8_t *ethFrame, int lenEthFrame);

uint32_t pDataAvailable(pQueue_t *pQ);
uint8_t devMem[ADIN1110_DEVICE_SIZE];

adin1110_DriverConfig_t drvConfig = { .pDevMem = (void*) devMem, .devMemSize =
		sizeof(devMem), .fcsCheckEn = true, };

adi_eth_LinkStatus_e linkStatus;
adi_eth_LinkStatus_e linkState;

static void txCallback(void *pCBParam, uint32_t Event, void *pArg) {
	adi_eth_BufDesc_t *desc = (adi_eth_BufDesc_t*) pArg;

	for (int i = 0; i < NUM_TX_DESC; i++) {
		if (&txBufDesc[i] == desc) {
			txBufAvailable[i] = true;
			return;
		}
	}
	DEBUG_MESSAGE("txCallback: WARNING! Descriptor not found!\n");
}
static void rxCallback(void *pCBParam, uint32_t Event, void *pArg) {
	adin1110_DeviceHandle_t hDevice = (adin1110_DeviceHandle_t) pCBParam;
	adi_eth_BufDesc_t *pRxBufDesc = (adi_eth_BufDesc_t*) pArg;
	uint16_t frmLen = pRxBufDesc->trxSize;
	uint8_t *payload = pRxBufDesc->pBuf;

	if (frmLen < 42) {
		LINK_STATS_INC(link.drop);
		adin1110_SubmitRxBuffer(hDevice, pRxBufDesc);
		return;
	}
	int unicast = ((payload[0] & 0x01) == 0);
	LINK_STATS_INC(link.recv);MIB2_STATS_NETIF_ADD(netif, ifinoctets, frmLen);
	if (unicast) {
		MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
	} else {
		MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
	}
	writePQ(&pQ[0], payload, frmLen);
	// Reinitialize the descriptor that triggered the callback.
	pRxBufDesc->bufSize = MAX_FRAME_BUF_SIZE;
	pRxBufDesc->cbFunc = rxCallback;
	// If necessary, you might also reset the buffer pointer here if it can change.
	// For example: pRxBufDesc->pBuf = <appropriate buffer pointer for this descriptor>;
	adin1110_SubmitRxBuffer(hDevice, pRxBufDesc);
}

void cbLinkChange(void *pCBParam, uint32_t Event, void *pArg) {
	adi_eth_LinkStatus_e linkStatus;
	linkStatus = *(adi_eth_LinkStatus_e*) pArg;

	if (linkStatus == ADI_ETH_LINK_STATUS_UP) {
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);
		DEBUG_MESSAGE("Ethernet Link Status: UP\r\n");
	} else {
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_RESET);
		DEBUG_MESSAGE("Ethernet Link Status: DOWN\r\n");
	}
}

uint32_t adi_phy_GetLinkStatus(uint8_t *status) {
	uint32_t result = 0;

	*status = (linkState == ADI_ETH_LINK_STATUS_UP) ? 1 : 0;
	return result;
}

uint32_t sys_now(void) {
	return BSP_SysNow();
}

err_t LwIP_ADIN1110LinkInput(struct netif *netif) {

	struct pbuf *p = (struct pbuf*) readPQ(&pQ[0]);
	if (p == NULL) {
#ifdef QUEUE_DEBUG
        DEBUG_MESSAGE("Failed to read packet from the queue.\r\n");
#endif
		return ERR_MEM;
	}

	if (netif->input(p, netif) != ERR_OK) {
		LWIP_DEBUGF(NETIF_DEBUG, ("IP input error\r\n"));
		pbuf_free(p);
		p = NULL;
	}

	return ERR_OK;
}

static err_t low_level_output(struct netif *netif, struct pbuf *p) {

	LwIP_ADIN1110_t *eth = (LwIP_ADIN1110_t*) netif->state;
	adin1110_DeviceHandle_t *hDevice = eth->hDevice;

	struct pbuf *pp;
	uint16_t frameLen = 0;
	int total_len = 0;

	for (pp = p, total_len = 0; pp != NULL; pp = pp->next) {
		frameLen = pp->len;

		if (pp->payload == NULL) {
			DEBUG_MESSAGE("ERROR: pbuf payload is NULL! Skipping segment.\n");
			continue;
		}

		if (frameLen < 2) {
			DEBUG_MESSAGE("WARNING: Skipping small frame of length %d\n",
					frameLen);
			continue;
		}

		memcpy(txBuf[txBufIndex] + total_len, (unsigned char*) pp->payload,
				frameLen);
		total_len += frameLen;

		if (total_len >= MAX_FRAME_BUF_SIZE) {
			DEBUG_MESSAGE(
					"ERROR: Frame too large! total_len=%d, MAX_FRAME_BUF_SIZE=%d\n",
					total_len, MAX_FRAME_BUF_SIZE);
			return ERR_VAL;
		}
	}

	LINK_STATS_INC(link.xmit);MIB2_STATS_NETIF_ADD(netif, ifoutoctets, total_len);

	if (total_len < MIN_FRAME_SIZE) {
		total_len = MIN_FRAME_SIZE;
	}

	txBufDesc[txBufIndex].pBuf = &txBuf[txBufIndex][0];
	txBufDesc[txBufIndex].trxSize = total_len;
	txBufDesc[txBufIndex].bufSize = MAX_FRAME_BUF_SIZE;
	txBufDesc[txBufIndex].egressCapt = ADI_MAC_EGRESS_CAPTURE_NONE;
	txBufDesc[txBufIndex].cbFunc = txCallback;

	if ((txBufDesc[txBufIndex].pBuf[0] & 1) != 0) {
		MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
	} else {
		MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
	}

	portENTER_CRITICAL();
	uint32_t attempts = 0;
	adi_eth_Result_e res = ADI_ETH_QUEUE_FULL;
	while (res == ADI_ETH_QUEUE_FULL) {
		res = adin1110_SubmitTxBuffer(*hDevice, &txBufDesc[txBufIndex]);
		if (res == ADI_ETH_QUEUE_FULL) {
			printf("Tx queue full, retrying...\n");
			osDelay(pdMS_TO_TICKS(1));
			attempts++;
			if (attempts > 1000) {
				DEBUG_MESSAGE("Tx queue stuck. Dropping frame.\n");
				LINK_STATS_INC(link.drop);
				portEXIT_CRITICAL();
				return ERR_MEM;
			}
		}
	}
	if (res != ADI_ETH_SUCCESS) {
		DEBUG_MESSAGE("SubmitTxBuffer failed with code=0x%08X\n", res);
		LINK_STATS_INC(link.drop);
		portEXIT_CRITICAL();
		return ERR_IF;
	}
	if (++txBufIndex >= NUM_TX_DESC) {
		txBufIndex = 0;
	}
	portEXIT_CRITICAL();
	return ERR_OK;
}

static err_t LwIP_ADIN1110LinkOutput(struct netif *netif, struct pbuf *p) {
	low_level_output(netif, p);
	return ERR_OK;
}

/* Not using SSI and HTTPD in build */
//static u16_t ssiHandler(const char *tag, char *insertBuffer,
//		int insertBufferLen) {
//	return 1;
//}

adi_eth_Result_e LwIP_StructInit(LwIP_ADIN1110_t *eth,
		adin1110_DeviceHandle_t *hDevice, uint8_t macAddress[6]) {
	eth->hDevice = hDevice;
	if (macAddress == NULL) {
		DEBUG_MESSAGE("Error: MAC Address is NULL\r\n");
		return ADI_ETH_INVALID_PARAM;
	}

	memcpy(eth->macAddress, macAddress, 6);
	DEBUG_MESSAGE("MAC Address initialized: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
			macAddress[0], macAddress[1], macAddress[2], macAddress[3],
			macAddress[4], macAddress[5]);

	return ADI_ETH_SUCCESS;
}

static err_t LwipADIN1110Init(struct netif *netif) {
	LwIP_ADIN1110_t *eth = (LwIP_ADIN1110_t*) netif->state;

	netif->output = etharp_output;
	netif->linkoutput = LwIP_ADIN1110LinkOutput;
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	netif->mtu = ETHERNET_MTU;
	netif->flags =
	NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_NETIF_HOSTNAME
	netif->hostname = HOSTNAME;
#endif

	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP
			| NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP;
	MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, NETIF_LINK_SPEED_IN_BPS);

	memcpy(netif->hwaddr, eth->macAddress, sizeof(netif->hwaddr));
	netif->hwaddr_len = sizeof(netif->hwaddr);

	return ERR_OK;
}

static adi_eth_Result_e ADIN1110Init(LwIP_ADIN1110_t *eth) {
	adi_eth_Result_e result;
	adin1110_DeviceHandle_t *hDevice = eth->hDevice;
	uint8_t brcstMAC[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	result = adin1110_AddAddressFilter(*hDevice, brcstMAC, NULL, 0);
	if (result != ADI_ETH_SUCCESS) {
		DEBUG_MESSAGE(
				"Error: Adding broadcast MAC address filter failed. Code: 0x%08X\r\n",
				result);
		return result;
	}
	result = adin1110_AddAddressFilter(*hDevice, eth->macAddress, NULL, 0);
	if (result != ADI_ETH_SUCCESS) {
		DEBUG_MESSAGE("Error: Adding device MAC address filter failed.\r\n");
		return result;
	}

	result = adin1110_SetCutThroughMode(*hDevice, true, true); // TXCTE = true, RXCTE = true
	if (result != ADI_ETH_SUCCESS) {
		DEBUG_MESSAGE("Error: Failed to set cut-through mode. Code: 0x%08X\r\n",
				result);
		return result;
	}
	bool txcteEnabled, rxcteEnabled;
	result = adin1110_GetCutThroughMode(*hDevice, &txcteEnabled, &rxcteEnabled);
	if (result == ADI_ETH_SUCCESS) {
		DEBUG_MESSAGE("Cut-through mode - TXCTE: %d, RXCTE: %d\r\n",
				txcteEnabled, rxcteEnabled);
	} else {
		DEBUG_MESSAGE("Error: Failed to get cut-through mode. Code: 0x%08X\r\n",
				result);
	}

	result = adin1110_WriteRegister(*hDevice, ADDR_MAC_TX_THRESH, 0x1);
	if (result != ADI_ETH_SUCCESS) {
		printf("Failed to set TX_THRESH: 0x%08X\n", result);
	}

	result = adin1110_SetChunkSize(*hDevice, ADI_MAC_OA_CPS_64BYTE);
	if (result != ADI_ETH_SUCCESS) {
		printf("Failed to set chunk size: 0x%08X\n", result);
	}

	uint32_t config0, txThresh;
	adin1110_ReadRegister(*hDevice, ADDR_MAC_CONFIG0, &config0);
	adin1110_ReadRegister(*hDevice, ADDR_MAC_TX_THRESH, &txThresh);
	printf("CONFIG0: 0x%08lX, TX_THRESH: 0x%08lX\n", (unsigned long) config0,
			(unsigned long) txThresh);

	result = adin1110_SyncConfig(*hDevice);
	if (result != ADI_ETH_SUCCESS) {
		DEBUG_MESSAGE("Error: Synchronizing configuration failed.\r\n");
		return result;
	}
	result = adin1110_RegisterCallback(*hDevice, cbLinkChange,
			ADI_MAC_EVT_LINK_CHANGE);
	if (result != ADI_ETH_SUCCESS) {
		DEBUG_MESSAGE("Error: Registering link change callback failed.\r\n");
		return result;
	}
	for (uint32_t i = 0; i < NUM_RX_DESC; i++) {
		rxBufDesc[i].pBuf = &rxBuf[i][0];
		rxBufDesc[i].bufSize = MAX_FRAME_BUF_SIZE;
		rxBufDesc[i].cbFunc = rxCallback;

		result = adin1110_SubmitRxBuffer(*hDevice, &rxBufDesc[i]);
		if (result != ADI_ETH_SUCCESS) {
			DEBUG_MESSAGE(
					"Error: SubmitRxBuffer() failed at i=%lu (code=0x%08X)\n",
					i, result);
			return result;
		}
	}
	for (uint32_t i = 0; i < NUM_TX_DESC; i++) {
		txBufAvailable[i] = true;
	}
	result = adin1110_Enable(*hDevice);
	if (result != ADI_ETH_SUCCESS) {
		DEBUG_MESSAGE("Error: Enabling device failed.\n");
		return result;
	}

	do {
		result = adin1110_GetLinkStatus(*hDevice, &linkStatus);
		DEBUG_RESULT("adin1110_GetLinkStatus", result, ADI_ETH_SUCCESS);
	} while (linkStatus != ADI_ETH_LINK_STATUS_UP);

	initPQueue(&pQ[0]);

	DEBUG_MESSAGE("ADIN1110 initialization completed successfully.\r\n");
	return result;
}

void LwIP_Init(LwIP_ADIN1110_t *eth, board_t *boardDetails) {
	ADIN1110Init(eth);
	lwip_init();

	/* Not using SSI and HTTPD in build */
//	http_set_ssi_handler(ssiHandler, NULL, 0);
//	httpd_init();

	if (boardDetails->ip_addr_fixed == 1) {
		ip4_addr_t ip, mask, gw;

		IP4_ADDR(&ip, boardDetails->ip_addr[0], boardDetails->ip_addr[1],
				boardDetails->ip_addr[2], boardDetails->ip_addr[3]);
		IP4_ADDR(&mask, boardDetails->net_mask[0], boardDetails->net_mask[1],
				boardDetails->net_mask[2], boardDetails->net_mask[3]);
		IP4_ADDR(&gw, boardDetails->gateway[0], boardDetails->gateway[1],
				boardDetails->gateway[2], boardDetails->gateway[3]);

		netif_add(&eth->netif, &ip, &mask, &gw, eth, LwipADIN1110Init,
				ethernet_input);

		netif_set_default(&eth->netif);
		netif_set_up(&eth->netif);
	} else {
		netif_add(&eth->netif, IPADDR_ANY, IPADDR_ANY, IPADDR_ANY, eth,
				LwipADIN1110Init, ethernet_input);

		netif_set_default(&eth->netif);
		netif_set_up(&eth->netif);
		dhcp_start(&eth->netif);
	}
}

void initPQueue(pQueue_t *pQ) {
	pQ->nWrQ = 0;
	pQ->nRdQ = 0;
}

uint32_t pDataAvailable(pQueue_t *pQ) {
	if (pQ->nWrQ != pQ->nRdQ) {
		return 1;
	}
	return 0;
}

void writePQ(pQueue_t *pQ, uint8_t *ethFrame, int lenEthFrame) {
	if (lenEthFrame > MAX_P_QUEUE_SZ) {
		DEBUG_MESSAGE("ERROR: Frame too large (%d bytes), dropping packet!\n",
				lenEthFrame);
		return;
	}
	int nextIndex = (pQ->nWrQ + 1) % MAX_P_QUEUE;
	if (nextIndex == pQ->nRdQ) {
		DEBUG_MESSAGE("ERROR: Queue full, dropping packet!\n");
		return;
	}
	memcpy(&pQ->pData[pQ->nWrQ][0], ethFrame, lenEthFrame);
	pQ->lenData[pQ->nWrQ] = lenEthFrame;
	pQ->nWrQ++;
	pQ->nWrQ %= MAX_P_QUEUE;
}

void* readPQ(pQueue_t *pQ) {
	if (pQ->nRdQ == pQ->nWrQ) {
#ifdef QUEUE_DEBUG
        DEBUG_MESSAGE("ERROR: Queue empty, no packet to read!\n");
#endif
		return NULL;
	}
	int ethFrmLen = pQ->lenData[pQ->nRdQ];
	if (ethFrmLen <= 0 || ethFrmLen > MAX_FRAME_BUF_SIZE) {
		DEBUG_MESSAGE("ERROR: Invalid frame length %d, skipping read\n",
				ethFrmLen);
		return NULL;
	}
	struct pbuf *p = pbuf_alloc(PBUF_RAW, ethFrmLen, PBUF_RAM);
	if (p == NULL) {
		DEBUG_MESSAGE("pbuf_alloc() failed, returning NULL\n");
		return NULL;
	}
	memcpy((uint8_t*) p->payload, &pQ->pData[pQ->nRdQ][0], ethFrmLen);
	pQ->nRdQ++;
	pQ->nRdQ %= MAX_P_QUEUE;
	return (void*) p;
}

uint32_t discoveradin1110(adin1110_DeviceHandle_t *hDevice) {
	adi_eth_Result_e result;
	uint32_t error = 1;

	/****** Driver Init *****/
	for (uint32_t i = 0; i < ADIN1110_INIT_ITER; i++) {
		result = adin1110_Init(*hDevice, &drvConfig);
		if (result == ADI_ETH_SUCCESS) {
			error = 0;
			break;
		}
	}
	return error;
}

#endif

