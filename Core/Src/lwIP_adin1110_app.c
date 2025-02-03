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
#include "adin1110.h"
#include "netif/etharp.h"
#include "lwip/ip_addr.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/arch.h"
#include "lwip/apps/httpd.h"
#include <lwip/inet.h>
#define ADIN1110_INIT_ITER  (5)
#define MAX_FRAME_BUF_SIZE  (MAX_FRAME_SIZE + 4 + 2 + 4)
#define FRAME_SIZE          (1518)
#define BUFF_DESC_COUNT     (5)

#define ETHERNET_MTU        (1500)

HAL_ALIGNED_PRAGMA(4)
static uint8_t rxBuf[BUFF_DESC_COUNT][MAX_FRAME_BUF_SIZE] HAL_ALIGNED_ATTRIBUTE(4);
static adi_eth_BufDesc_t rxBufDesc[BUFF_DESC_COUNT];

HAL_ALIGNED_PRAGMA(4)
static uint8_t txBuf[BUFF_DESC_COUNT][MAX_FRAME_BUF_SIZE] HAL_ALIGNED_ATTRIBUTE(4);

adi_eth_BufDesc_t txBufDesc[BUFF_DESC_COUNT];
bool txBufAvailable[BUFF_DESC_COUNT];
int txBufIndex = 0;


HAL_ALIGNED_PRAGMA(4)
pQueue_t pQ[MAX_PQ] HAL_ALIGNED_ATTRIBUTE(4);

#define IFNAME0         'e'
#define IFNAME1         '0'
#define HOSTNAME         "ADI_10BASE-T1L_Demo"
#define NETIF_LINK_SPEED_IN_BPS 10000000

static void initPQueue(pQueue_t *pQ);

static void*           readPQ(pQueue_t* pQ);
static void writePQ(pQueue_t *pQ, uint8_t *ethFrame, int lenEthFrame);
static uint32_t pDataAvailable(pQueue_t *pQ);

uint8_t devMem[ADIN1110_DEVICE_SIZE];

adin1110_DriverConfig_t drvConfig = { .pDevMem = (void*) devMem, .devMemSize =
		sizeof(devMem), .fcsCheckEn = false, };

adi_eth_LinkStatus_e linkStatus;

bool linkStatusChanged;
adi_eth_LinkStatus_e linkState;

#ifdef TCP_IP_DEBUG
static inline uint16_t swap16(uint16_t val) {
	return __builtin_bswap16(val);
}

static inline uint32_t swap32(uint32_t val) {
	return __builtin_bswap32(val);
}
#endif /* TCP/IP_DEBUG */

static void txCallback(void *pCBParam, uint32_t Event, void *pArg)
{
    txBufAvailable[0] = true;
}
static void rxCallback(void *pCBParam, uint32_t Event, void *pArg) {
	adin1110_DeviceHandle_t hDevice = (adin1110_DeviceHandle_t) pCBParam;
	adi_eth_BufDesc_t *pRxBufDesc = (adi_eth_BufDesc_t*) pArg;
	uint16_t frmLen = pRxBufDesc->trxSize;
	uint8_t *payload = pRxBufDesc->pBuf;

#ifdef TCP_IP_DEBUG
		DEBUG_MESSAGE("[DEBUG] RX Buffer Descriptor Address: %p\n", pRxBufDesc);
		DEBUG_MESSAGE("[DEBUG] RX Buffer Address: %p\n", payload);
		DEBUG_MESSAGE("[DEBUG] Dumping First 64 Bytes of RX Buffer:\n");
		for (int i = 0; i < 64 && i < frmLen; i++) {
			DEBUG_MESSAGE("0x%02X ", payload[i]);
			if ((i + 1) % 16 == 0)
				DEBUG_MESSAGE("\n");
		}
		DEBUG_MESSAGE("\n");
		DEBUG_MESSAGE("[DEBUG] Corrected Frame Dump (%d bytes):\n", frmLen);
		for (int i = 0; i < frmLen; i++) {
			DEBUG_MESSAGE("0x%02X ", payload[i]);
			if ((i + 1) % 16 == 0)
				DEBUG_MESSAGE("\n");
		}
		DEBUG_MESSAGE("\n");
		uint16_t ethType = swap16(*(uint16_t*) &payload[12]);

		DEBUG_MESSAGE("    Dest MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", payload[0],
				payload[1], payload[2], payload[3], payload[4], payload[5]);
		DEBUG_MESSAGE("    Src MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", payload[6],
				payload[7], payload[8], payload[9], payload[10], payload[11]);

		if (ethType == 0x0806) {
			uint16_t arp_hwtype = swap16(*(uint16_t*) &payload[14]);
			uint16_t arp_proto = swap16(*(uint16_t*) &payload[16]);
			uint8_t arp_hwlen = payload[18];
			uint8_t arp_protolen = payload[19];
			uint16_t arp_opcode = swap16(*(uint16_t*) &payload[20]);

			uint8_t sender_mac[6];
			memcpy(sender_mac, &payload[22], 6);

			uint32_t sender_ip = swap16(*(uint32_t*) &payload[28]);

			uint8_t target_mac[6];
			memcpy(target_mac, &payload[32], 6);

			uint32_t target_ip = swap16(*(uint32_t*) &payload[38]);

			DEBUG_MESSAGE(
					"[ARP] Parsed Values - HW Type: 0x%04X, Proto: 0x%04X, HW Len: %u, Protolen: %u, Opcode: 0x%04X\n",
					arp_hwtype, arp_proto, arp_hwlen, arp_protolen, arp_opcode);
			DEBUG_MESSAGE(
					"[ARP] Sender MAC: %02X:%02X:%02X:%02X:%02X:%02X, Sender IP: %d.%d.%d.%d\n",
					sender_mac[0], sender_mac[1], sender_mac[2], sender_mac[3],
					sender_mac[4], sender_mac[5], (sender_ip >> 24) & 0xFF,
					(sender_ip >> 16) & 0xFF, (sender_ip >> 8) & 0xFF,
					sender_ip & 0xFF);
			DEBUG_MESSAGE(
					"[ARP] Target MAC: %02X:%02X:%02X:%02X:%02X:%02X, Target IP: %d.%d.%d.%d\n",
					target_mac[0], target_mac[1], target_mac[2], target_mac[3],
					target_mac[4], target_mac[5], (target_ip >> 24) & 0xFF,
					(target_ip >> 16) & 0xFF, (target_ip >> 8) & 0xFF,
					target_ip & 0xFF);

			if (arp_hwtype != 0x0001 || arp_proto != 0x0800 || arp_hwlen != 6
					|| arp_protolen != 4) {
				DEBUG_MESSAGE(
						"[ERROR] Invalid ARP packet format! Dropping packet.\n");
				adin1110_SubmitRxBuffer(hDevice, pRxBufDesc);
				return;
			}
	DEBUG_MESSAGE("[DEBUG] RX Buffer Descriptor Address: %p\n", pRxBufDesc);
	DEBUG_MESSAGE("[DEBUG] RX Buffer Address: %p\n", payload);
	DEBUG_MESSAGE("[DEBUG] Dumping First 64 Bytes of RX Buffer:\n");
	for (int i = 0; i < 64 && i < frmLen; i++) {
		DEBUG_MESSAGE("0x%02X ", payload[i]);
		if ((i + 1) % 16 == 0)
			DEBUG_MESSAGE("\n");
	}
	DEBUG_MESSAGE("\n");
	DEBUG_MESSAGE("[DEBUG] Corrected Frame Dump (%d bytes):\n", frmLen);
	for (int i = 0; i < frmLen; i++) {
		DEBUG_MESSAGE("0x%02X ", payload[i]);
		if ((i + 1) % 16 == 0)
			DEBUG_MESSAGE("\n");
	}
	DEBUG_MESSAGE("\n");

	uint16_t ethType = swap32(*(uint16_t*) &payload[12]);
	DEBUG_MESSAGE("[RX] Incoming Ethernet Frame (Len=%d, EtherType=0x%04X)\n",
				frmLen, ethType);

	DEBUG_MESSAGE("[RX] Incoming Ethernet Frame (Len=%d, EtherType=0x%04X)\n",
			frmLen, ethType);
	DEBUG_MESSAGE("    Dest MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", payload[0],
			payload[1], payload[2], payload[3], payload[4], payload[5]);
	DEBUG_MESSAGE("    Src MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", payload[6],
			payload[7], payload[8], payload[9], payload[10], payload[11]);

		if (ethType == 0x0806) {
	 uint16_t arp_hwtype = swap32(*(uint16_t*) &payload[14]);
	 uint16_t arp_proto = swap32(*(uint16_t*) &payload[16]);
	 uint8_t arp_hwlen = payload[18];
	 uint8_t arp_protolen = payload[19];
	 uint16_t arp_opcode = swap32(*(uint16_t*) &payload[20]);

	 uint8_t sender_mac[6];
	 memcpy(sender_mac, &payload[22], 6);

	 uint32_t sender_ip = swap32(*(uint32_t*) &payload[28]);

	 uint8_t target_mac[6];
	 memcpy(target_mac, &payload[32], 6);

	 uint32_t target_ip = swap32(*(uint32_t*) &payload[38]);


		DEBUG_MESSAGE(
				"[ARP] Parsed Values - HW Type: 0x%04X, Proto: 0x%04X, HW Len: %u, Protolen: %u, Opcode: 0x%04X\n",
				arp_hwtype, arp_proto, arp_hwlen, arp_protolen, arp_opcode);
		DEBUG_MESSAGE(
				"[ARP] Sender MAC: %02X:%02X:%02X:%02X:%02X:%02X, Sender IP: %d.%d.%d.%d\n",
				sender_mac[0], sender_mac[1], sender_mac[2], sender_mac[3],
				sender_mac[4], sender_mac[5], (sender_ip >> 24) & 0xFF,
				(sender_ip >> 16) & 0xFF, (sender_ip >> 8) & 0xFF,
				sender_ip & 0xFF);
		DEBUG_MESSAGE(
				"[ARP] Target MAC: %02X:%02X:%02X:%02X:%02X:%02X, Target IP: %d.%d.%d.%d\n",
				target_mac[0], target_mac[1], target_mac[2], target_mac[3],
				target_mac[4], target_mac[5], (target_ip >> 24) & 0xFF,
				(target_ip >> 16) & 0xFF, (target_ip >> 8) & 0xFF,
				target_ip & 0xFF);

	if (arp_hwtype != 0x0001 || arp_proto != 0x0800 || arp_hwlen != 6
	 || arp_protolen != 4) {
	 DEBUG_MESSAGE(
	 "[ERROR] Invalid ARP packet format! Dropping packet.\n");
	 adin1110_SubmitRxBuffer(hDevice, pRxBufDesc);
	 return;
	 }

	 if (sender_ip == 0) {
	 DEBUG_MESSAGE("[ARP] Ignoring ARP packet with sender IP 0.0.0.0\n");
	 adin1110_SubmitRxBuffer(hDevice, pRxBufDesc);
	 return;
	 }

	 DEBUG_MESSAGE("[ARP] Processing ARP packet in lwIP.\n");
	 }
#endif /* TCP/IP_DEBUG */

	int unicast = ((payload[0] & 0x01) == 0);
	LINK_STATS_INC(link.recv); MIB2_STATS_NETIF_ADD(netif, ifinoctets, frmLen);
	if (unicast) {
		MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
	} else {
		MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
	}
	writePQ(&pQ[0], payload, frmLen);
	struct pbuf *p = pbuf_alloc(PBUF_RAW, frmLen, PBUF_POOL);
	if (p == NULL) {

		DEBUG_MESSAGE(
				"[ARP] Failed to allocate pbuf for lwIP processing!\n");

		adin1110_SubmitRxBuffer(hDevice, pRxBufDesc);
		return;
	}

	if (pbuf_take(p, payload, frmLen) != ERR_OK) {
		DEBUG_MESSAGE("[rxCallback] pbuf_take failed, dropping frame.\n");
		pbuf_free(p);
		adin1110_SubmitRxBuffer(hDevice, pRxBufDesc);
		return;
	}

	err_t err = netif_input(p, &myConn.netif);
	if (err != ERR_OK) {
		DEBUG_MESSAGE("[rxCallback] netif_input error: %d\n", err);
		pbuf_free(p);
	}
	rxBufDesc[0].pBuf = &rxBuf[0][0];
	rxBufDesc[0].bufSize = MAX_FRAME_BUF_SIZE;
	rxBufDesc[0].cbFunc = rxCallback;

	adin1110_SubmitRxBuffer(hDevice, pRxBufDesc);
}

void cbLinkChange(void *pCBParam, uint32_t Event, void *pArg) {
	adi_eth_LinkStatus_e linkStatus;
	linkStatus = *(adi_eth_LinkStatus_e*) pArg;

	if (linkStatus == ADI_ETH_LINK_STATUS_UP) {
		DEBUG_MESSAGE("Ethernet Link Status: UP\r\n");
	} else {
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
#ifdef TCP_IP_DEBUG
void print_lwip_arp_table(void) {
	printf("\n[DEBUG] Current ARP Table:\n");
	for (int i = 0; i < ARP_TABLE_SIZE; i++) {
		ip4_addr_t *ipaddr;
		struct netif *netif_out;
		struct eth_addr *macaddr;
		if (etharp_get_entry(i, &ipaddr, &netif_out, &macaddr) == ERR_OK) {
			if (ipaddr && macaddr) {
				printf(
						"[%lu] ARP %d: IP=%s, MAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
						BSP_SysNow(), i, ip4addr_ntoa(ipaddr), macaddr->addr[0],
						macaddr->addr[1], macaddr->addr[2], macaddr->addr[3],
						macaddr->addr[4], macaddr->addr[5]);

				if ((ntohl(ipaddr->addr) & 0xFFFFFF00) != 0xC0A80100) {

					printf("[WARNING] Unexpected ARP entry: %s\n",
							ip4addr_ntoa(ipaddr));
				}
			}
		}
	}
}
#endif /* TCP/IP_DEBUG */

err_t LwIP_ADIN1110LinkInput(struct netif *netif) {
	if (pDataAvailable(&pQ[0]) == 0) {
		return ERR_OK;
	}

	struct pbuf *p = (struct pbuf*) readPQ(&pQ[0]);
	if (p == NULL) {
		DEBUG_MESSAGE("Failed to read packet from the queue.\r\n");
		return ERR_MEM;
	}

#ifdef TCP_IP_DEBUG
	uint8_t *payload = (uint8_t*) p->payload;
	uint16_t ethType = (payload[12] << 8) | payload[13];
	DEBUG_MESSAGE("\n==============================================");
	DEBUG_MESSAGE("\nSTART OF PACKET");
	DEBUG_MESSAGE("\nPacket Type: 0x%04X, Length: %d\n", ethType, p->len);

	for (int i = 0; i < p->len; i++) {
		DEBUG_MESSAGE("0x%02X ", payload[i]);
		if ((i + 1) % 16 == 0)
			DEBUG_MESSAGE("\n");
	}
	DEBUG_MESSAGE("\n");


	if (ethType == 0x0800) {
		uint8_t ipProtocol = payload[23];

		if (ipProtocol == 17) {
			uint16_t udpSrcPort = (payload[34] << 8) | payload[35];
			uint16_t udpDstPort = (payload[36] << 8) | payload[37];

			if (udpDstPort == 67 || udpDstPort == 68) {
				DEBUG_MESSAGE(
						"🔹 [DHCP] DHCP Packet Detected! (UDP Port: %d)",
						udpDstPort);
			} else {
				DEBUG_MESSAGE(
						"🔹 [UDP] Non-DHCP UDP Packet (Src: %d, Dst: %d)",
						udpSrcPort, udpDstPort);
			}
		} else if (ipProtocol == 1) {
			DEBUG_MESSAGE("🔹 [ICMP] ICMP Packet Detected!");
		} else {
			DEBUG_MESSAGE("🔹 [IPv4] Other IPv4 Packet (Protocol: 0x%02X)",
					ipProtocol);
		}
	} else if (ethType == 0x0806) {
		DEBUG_MESSAGE("🔹 [ARP] ARP Packet Detected!");
	} else {
			DEBUG_MESSAGE("🔹 [UNKNOWN] Unrecognized Packet Type: 0x%04X",
				ethType);
	}
	DEBUG_MESSAGE("\nEND OF PACKET");
	DEBUG_MESSAGE("\n==============================================\n");
#endif /* TCP/IP_DEBUG */
	if (netif->input(p, netif) != ERR_OK) {
		LWIP_DEBUGF(NETIF_DEBUG, ("IP input error\r\n"));
		pbuf_free(p);
		p = NULL;
	}

	return ERR_OK;
}

static err_t low_level_output(struct netif *netif, struct pbuf *p) {

#ifdef TCP_IP_DEBUG
	uint16_t ethType = ((uint8_t*) p->payload)[12] << 8
			| ((uint8_t*) p->payload)[13];
#endif /* TCP/IP_DEBUG */

	LwIP_ADIN1110_t *eth = (LwIP_ADIN1110_t*) netif->state;
	adin1110_DeviceHandle_t *hDevice = eth->hDevice;

	struct pbuf *pp;
	uint16_t frameLen = 0;
	int total_len = 0;

#ifdef TCP_IP_DEBUG
	DEBUG_MESSAGE("\n==============================================");
	DEBUG_MESSAGE("\nSTART OF TRANSMIT PACKET");
	DEBUG_MESSAGE("\nPacket Type: 0x%04X, Length: %d\n", ethType, p->tot_len);
#endif /* TCP/IP_DEBUG */

	for (pp = p, total_len = 0; pp != NULL; pp = pp->next) {
		frameLen = pp->len;

		if (frameLen < 2) {
			continue;
		}

		memcpy(txBuf[txBufIndex] + total_len, (unsigned char*) pp->payload,
				frameLen);
		total_len += frameLen;

		if (total_len >= MAX_FRAME_BUF_SIZE) {
			return ERR_VAL;
		}
	}


#ifdef TCP_IP_DEBUG
	uint8_t *payload = (uint8_t*) p->payload;
	for (int i = 0; i < p->tot_len; i++) {
		DEBUG_MESSAGE("0x%02X ", payload[i]);
		if ((i + 1) % 16 == 0)
			DEBUG_MESSAGE("\n");
	}
	DEBUG_MESSAGE("\n");
	DEBUG_MESSAGE("\nEND OF TRANSMIT PACKET");
	DEBUG_MESSAGE("\n==============================================\n");
#endif /* TCP/IP_DEBUG */

	LINK_STATS_INC(link.xmit);
	MIB2_STATS_NETIF_ADD(netif, ifoutoctets, total_len);

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
	while (adin1110_SubmitTxBuffer(*hDevice, &txBufDesc[txBufIndex])
			== ADI_ETH_QUEUE_FULL) {
		;;
	}

	if (txBufIndex++ >= 1) {
		txBufIndex = 0;
	}
	return ERR_OK;
}

static err_t LwIP_ADIN1110LinkOutput(struct netif *netif, struct pbuf *p) {
	low_level_output(netif, p);
	return ERR_OK;
}

static u16_t ssiHandler(const char *tag, char *insertBuffer,
		int insertBufferLen) {
	return 1;
}

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
		DEBUG_MESSAGE(
				"Error: Adding device MAC address filter failed.\r\n");
		return result;
	}
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
	for (uint32_t i = 0; i < 1; i++) {
		txBufAvailable[i] = true;

		rxBufDesc[i].pBuf = &rxBuf[i][0];
		rxBufDesc[i].bufSize = MAX_FRAME_BUF_SIZE;
		rxBufDesc[i].cbFunc = rxCallback;

		result = adin1110_SubmitRxBuffer(*hDevice, &rxBufDesc[i]);
	}
	result = adin1110_Enable(*hDevice);
	if (result != ADI_ETH_SUCCESS) {
		DEBUG_MESSAGE("Error: Enabling device failed.\r\n");
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
	http_set_ssi_handler(ssiHandler, NULL, 0);
	httpd_init();
	if (boardDetails->ip_addr_fixed == 1) {
		ip4_addr_t ip, mask, gw;

		IP4_ADDR(&ip, boardDetails->ip_addr[0], boardDetails->ip_addr[1],
				boardDetails->ip_addr[2], boardDetails->ip_addr[3]);
		IP4_ADDR(&mask, boardDetails->net_mask[0],
				boardDetails->net_mask[1], boardDetails->net_mask[2],
				boardDetails->net_mask[3]);
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
		DEBUG_MESSAGE(
				"[ERROR] Packet size exceeds buffer limit! Dropping packet of length %d\r\n",
				lenEthFrame);
		return;
	}

	if ((pQ->nWrQ + 1) % MAX_P_QUEUE == pQ->nRdQ) {
		DEBUG_MESSAGE(
				"[WARNING] Queue overflow: overwriting oldest packet\r\n");
		pQ->nRdQ = (pQ->nRdQ + 1) % MAX_P_QUEUE;
	}

	memcpy(&pQ->pData[pQ->nWrQ][0], ethFrame, lenEthFrame);
	pQ->lenData[pQ->nWrQ] = lenEthFrame;
#ifdef TCP_IP_DEBUG
	DEBUG_MESSAGE("[QUEUE] Packet enqueued at index %d, length: %d\r\n",
			pQ->nWrQ, lenEthFrame);
#endif /* TCP/IP_DEBUG */

	pQ->nWrQ = (pQ->nWrQ + 1) % MAX_P_QUEUE;

#ifdef TCP_IP_DEBUG
	DEBUG_MESSAGE("[QUEUE] Queue state after enqueue: nWrQ=%d, nRdQ=%d\r\n",
			pQ->nWrQ, pQ->nRdQ);

#endif /* TCP/IP_DEBUG */

}

void* readPQ(pQueue_t* pQ) {
	if (pQ->nWrQ == pQ->nRdQ) {
		DEBUG_MESSAGE("Queue underflow: no packets to dequeue\r\n");
		return NULL;
	}
	int ehtFrmLen = pQ->lenData[pQ->nRdQ];
	struct pbuf *p = pbuf_alloc(PBUF_RAW, MAX_FRAME_BUF_SIZE, PBUF_RAM);
	if (p == NULL) {
		DEBUG_MESSAGE("Failed to allocate pbuf for packet of length %d\r\n",
				ehtFrmLen);
		return NULL;
	}
	memcpy(((uint8_t*) p->payload), &pQ->pData[pQ->nRdQ][0], ehtFrmLen);

#ifdef TCP_IP_DEBUG
	DEBUG_MESSAGE("Packet dequeued from index %d, length: %d\r\n", pQ->nRdQ,
			ehtFrmLen);
#endif /* TCP/IP_DEBUG */

	pQ->nRdQ++;
	pQ->nRdQ %= MAX_P_QUEUE;

#ifdef TCP_IP_DEBUG
	DEBUG_MESSAGE("Queue state after dequeue: nWrQ=%d, nRdQ=%d\r\n",
			pQ->nWrQ, pQ->nRdQ);
#endif /* TCP/IP_DEBUG */

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

