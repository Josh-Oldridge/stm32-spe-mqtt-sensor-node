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
#include "lwip/udp.h"

#define ADIN1110_INIT_ITER  (5)
#define MAX_FRAME_BUF_SIZE  (MAX_FRAME_SIZE + 4 + 2)
#define FRAME_SIZE          (1518)
#define BUFF_DESC_COUNT     (4)

#define ETHERNET_MTU        (1500)

HAL_ALIGNED_PRAGMA(4)
static uint8_t          rxBuf[BUFF_DESC_COUNT][MAX_FRAME_BUF_SIZE] HAL_ALIGNED_ATTRIBUTE(4);
static adi_eth_BufDesc_t       rxBufDesc[BUFF_DESC_COUNT];

HAL_ALIGNED_PRAGMA(4)
static uint8_t          txBuf[BUFF_DESC_COUNT][MAX_FRAME_BUF_SIZE] HAL_ALIGNED_ATTRIBUTE(4);


adi_eth_BufDesc_t       txBufDesc[BUFF_DESC_COUNT];
bool                    txBufAvailable[BUFF_DESC_COUNT];
int                     txBufIndex = 0;

#define MAX_PQ 1
HAL_ALIGNED_PRAGMA(4)
pQueue_t pQ[MAX_PQ] HAL_ALIGNED_ATTRIBUTE(4);;

#define IFNAME0         'e'
#define IFNAME1         '0'
#define HOSTNAME         "ADI_10BASE-T1L_Demo"
#define NETIF_LINK_SPEED_IN_BPS 10000000

static void            initPQueue(pQueue_t* pQ);
static void*           readPQ(pQueue_t* pQ);
static void            writePQ(pQueue_t* pQ, uint8_t *ethFrame, int lenEthFrame);
static uint32_t        pDataAvailable(pQueue_t* pQ);

uint8_t devMem[ADIN1110_DEVICE_SIZE];

adin1110_DriverConfig_t drvConfig = {
    .pDevMem    = (void *)devMem,
    .devMemSize = sizeof(devMem),
    .fcsCheckEn = false,
};

adi_eth_LinkStatus_e    linkStatus;


bool linkStatusChanged;
adi_eth_LinkStatus_e linkState ;



#define QUERY_TIMEOUT 60000

static const char queryMsg[]       = "CMD:QUERY:LD1_ON?";
static const char responseOnMsg[]  = "CMD:RESPONSE:LD1_ON";
static const char responseOffMsg[] = "CMD:RESPONSE:LD1_OFF";

static ip4_addr_t remoteIP;
#define REMOTE_UDP_PORT 5000
#define LOCAL_UDP_PORT  5001
volatile QueryState_t queryState = STATE_IDLE;
volatile uint32_t querySentTime = 0;
static struct udp_pcb *query_udp_pcb = NULL;

#ifdef USE_LWIP

static void txCallback(void *pCBParam, uint32_t Event, void *pArg)
{
    txBufAvailable[0] = true;
}

static void rxCallback(void *pCBParam, uint32_t Event, void *pArg)
{
    adin1110_DeviceHandle_t hDevice = (adin1110_DeviceHandle_t)pCBParam;
    adi_eth_BufDesc_t *pRxBufDesc   = (adi_eth_BufDesc_t *)pArg;
    uint16_t frmLen                = pRxBufDesc->trxSize;
    if (frmLen > 18)
    {
        char receivedCmd[128] = {0};
        size_t copyLen = (frmLen - 18 < sizeof(receivedCmd))
                           ? (frmLen - 18)
                           : (sizeof(receivedCmd) - 1);

        memcpy(receivedCmd, &pRxBufDesc->pBuf[18], copyLen);
        DEBUG_MESSAGE("Received command: %s\r\n", receivedCmd);
        if (strncmp(receivedCmd, responseOnMsg, strlen(responseOnMsg)) == 0)
        {
            queryState = STATE_RESPONSE_RECEIVED;
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
            DEBUG_MESSAGE("LD1 turned ON\r\n");
        }
        else if (strncmp(receivedCmd, responseOffMsg, strlen(responseOffMsg)) == 0)
        {
            queryState = STATE_RESPONSE_RECEIVED;
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
            DEBUG_MESSAGE("LD1 turned OFF\r\n");
        }
    }
    writePQ(&pQ[0], pRxBufDesc->pBuf, frmLen);
    adin1110_SubmitRxBuffer(hDevice, pRxBufDesc);
}

void cbLinkChange(void *pCBParam, uint32_t Event, void *pArg)
{
    adi_eth_LinkStatus_e    linkStatus;

    linkStatus = *(adi_eth_LinkStatus_e *)pArg;
    linkState = linkStatus;
    linkStatusChanged = true;
    (void)linkStatus;
}

#endif


uint32_t adi_phy_GetLinkStatus(uint8_t *status)
{
    uint32_t    result = 0;

    *status = (linkState ==  ADI_ETH_LINK_STATUS_UP)?  1: 0;
    return result;
}


uint32_t sys_now(void)
{
  return BSP_SysNow();
}

static void udp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                                const ip_addr_t *addr, u16_t port)
{
    char recv_buf[128] = {0};
    if (p != NULL) {
        size_t copy_len = (p->len < sizeof(recv_buf)-1) ? p->len : sizeof(recv_buf)-1;
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
        }
        pbuf_free(p);
    }
}


err_t udp_send_query(void)
{
    struct pbuf *p;
    err_t err;
    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(queryMsg) - 1, PBUF_RAM);
    if (p == NULL) {
        DEBUG_MESSAGE("Failed to allocate pbuf for UDP query\r\n");
        return ERR_MEM;
    }
    memcpy(p->payload, queryMsg, sizeof(queryMsg) - 1);
    if (query_udp_pcb == NULL) {
        query_udp_pcb = udp_new();
        if (query_udp_pcb == NULL) {
            DEBUG_MESSAGE("Failed to create UDP PCB\r\n");
            pbuf_free(p);
            return ERR_MEM;
        }
        err = udp_bind(query_udp_pcb, IP4_ADDR_ANY, LOCAL_UDP_PORT);
        if (err != ERR_OK) {
            DEBUG_MESSAGE("UDP bind failed: %d\r\n", err);
            pbuf_free(p);
            return err;
        }
        udp_recv(query_udp_pcb, udp_recv_callback, NULL);
    }
    IP4_ADDR(&remoteIP, 192, 168, 1, 12);
    err = udp_sendto(query_udp_pcb, p, &remoteIP, REMOTE_UDP_PORT);
    if (err == ERR_OK) {
        querySentTime = BSP_SysNow();
        queryState = STATE_WAITING_FOR_RESPONSE;
        DEBUG_MESSAGE("UDP Query sent at %lu ms\r\n", querySentTime);
    } else {
        DEBUG_MESSAGE("UDP send failed: %d\r\n", err);
    }
    pbuf_free(p);
    return err;
}

void process_udp_query(void)
{
    uint32_t now = BSP_SysNow();
    if (queryState == STATE_WAITING_FOR_RESPONSE) {
        if ((now - querySentTime) >= QUERY_TIMEOUT) {
            DEBUG_MESSAGE("UDP Query timeout reached (%lu ms); resending query\r\n", (unsigned long)QUERY_TIMEOUT);
            udp_send_query();
        }
    } else if (queryState == STATE_IDLE) {
        udp_send_query();
    } else if (queryState == STATE_RESPONSE_RECEIVED) {
        queryState = STATE_IDLE;
    }
}



err_t LwIP_ADIN1110LinkInput(struct netif *netif)
{
    if (pDataAvailable(&pQ[0]) == 0)
    {
      return ERR_OK;
    }
    else
    {
		struct pbuf *p = (struct pbuf*) readPQ(&pQ[0]);
		if (p == NULL) {
			return ERR_MEM;
		}

		uint8_t *payload = (uint8_t*) p->payload;
		uint16_t ethType = (payload[12] << 8) | payload[13];
		if (ethType == 0x0800) {
			uint8_t ipProtocol = payload[23];
			if (ipProtocol == 0x01) {
				DEBUG_MESSAGE("ICMP packet passed to LWIP\r\n");
			}
		}

		if (netif->input(p, netif) != ERR_OK) {
			LWIP_DEBUGF(NETIF_DEBUG, ("IP input error\r\n"));
			pbuf_free(p);
			p = NULL;
		}
    }
   return  ERR_OK;
}


static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
   LwIP_ADIN1110_t* eth = (LwIP_ADIN1110_t*) netif->state;

    adin1110_DeviceHandle_t hDevice =   eth->hDevice;

    struct pbuf *pp;
    uint16_t frameLen = 0;
    int total_len = 0;

    for(pp = p, total_len = 0; pp != NULL; pp = pp->next)
    {
      frameLen =  pp->len ;

      if(frameLen < 2)
      {
        continue;
      }

      memcpy(txBuf[txBufIndex] + total_len  ,(unsigned char*) pp->payload, frameLen);
      total_len += frameLen ;

      if(total_len >= MAX_FRAME_BUF_SIZE)
      {
        return ERR_VAL;
      }
    }

    LINK_STATS_INC(link.xmit);
    MIB2_STATS_NETIF_ADD(netif, ifoutoctets, total_len);

    if(total_len < MIN_FRAME_SIZE)
    {
      total_len = MIN_FRAME_SIZE;
    }

    txBufDesc[txBufIndex].pBuf = &txBuf[txBufIndex][0];
    txBufDesc[txBufIndex].trxSize = total_len;
    txBufDesc[txBufIndex].bufSize = MAX_FRAME_BUF_SIZE;
    txBufDesc[txBufIndex].egressCapt = ADI_MAC_EGRESS_CAPTURE_NONE;
    txBufDesc[txBufIndex].cbFunc = txCallback;

    if ((txBufDesc[txBufIndex].pBuf[0] & 1) != 0)
    {
      MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
    }
    else
    {
      MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
    }

    while(adin1110_SubmitTxBuffer(hDevice, &txBufDesc[txBufIndex]) == ADI_ETH_QUEUE_FULL)
    {
      ;;
    }

    if(txBufIndex ++ >= 1 )
    {
      txBufIndex = 0;
    }
   return ERR_OK;
}

static err_t LwIP_ADIN1110LinkOutput(struct netif *netif, struct pbuf *p)
{
    low_level_output(netif, p);
    return ERR_OK;
}


static u16_t ssiHandler(const char* tag, char *insertBuffer, int insertBufferLen)
{
  return 1;
}


adi_eth_Result_e LwIP_StructInit(LwIP_ADIN1110_t* eth, adin1110_DeviceHandle_t hDevice, uint8_t macAddress[6])
{
    eth->hDevice = hDevice;

    if (macAddress == NULL) {
        DEBUG_MESSAGE("Error: MAC Address is NULL\r\n");
        return ADI_ETH_INVALID_PARAM;
    }

    memcpy(eth->macAddress, macAddress, 6);
    DEBUG_MESSAGE("MAC Address initialized: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                  macAddress[0], macAddress[1], macAddress[2],
                  macAddress[3], macAddress[4], macAddress[5]);

    return ADI_ETH_SUCCESS;
}

static err_t LwipADIN1110Init(struct netif *netif)
{
   LwIP_ADIN1110_t* eth = (LwIP_ADIN1110_t*) netif->state;

   netif->output = etharp_output;
   netif->linkoutput = LwIP_ADIN1110LinkOutput;
   netif->name[0] = IFNAME0;
   netif->name[1] = IFNAME1;
   netif->mtu = ETHERNET_MTU;
   netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	#if LWIP_NETIF_HOSTNAME
    netif->hostname = HOSTNAME;
	#endif


   netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP;
   MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, NETIF_LINK_SPEED_IN_BPS);

   memcpy(netif->hwaddr, eth->macAddress, sizeof(netif->hwaddr));
   netif->hwaddr_len = sizeof(netif->hwaddr);

   return ERR_OK;
}


static adi_eth_Result_e ADIN1110Init(LwIP_ADIN1110_t* eth)
{
    adi_eth_Result_e result;
    adin1110_DeviceHandle_t hDevice = eth->hDevice;
    uint8_t brcstMAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    result = discoveradin1110(hDevice);
    if (result != ADI_ETH_SUCCESS)
    {
        DEBUG_MESSAGE("Error: ADIN1110 discovery failed.\r\n");
        return result;
    }
    result = adin1110_AddAddressFilter(hDevice, brcstMAC, NULL, 0);
    if (result != ADI_ETH_SUCCESS)
    {
        DEBUG_MESSAGE("Error: Adding broadcast MAC address filter failed. Code: 0x%08X\r\n", result);
        return result;
    }
    result = adin1110_AddAddressFilter(hDevice, eth->macAddress, NULL, 0);
    if (result != ADI_ETH_SUCCESS)
    {
        DEBUG_MESSAGE("Error: Adding device MAC address filter failed.\r\n");
        return result;
    }
    result = adin1110_SyncConfig(hDevice);
    if (result != ADI_ETH_SUCCESS)
    {
        DEBUG_MESSAGE("Error: Synchronizing configuration failed.\r\n");
        return result;
    }
    result = adin1110_RegisterCallback(hDevice, cbLinkChange, ADI_MAC_EVT_LINK_CHANGE);
    if (result != ADI_ETH_SUCCESS)
    {
        DEBUG_MESSAGE("Error: Registering link change callback failed.\r\n");
        return result;
    }
    for (uint32_t i = 0; i < BUFF_DESC_COUNT; i++)
    {
        txBufAvailable[i] = true;

        rxBufDesc[i].pBuf = &rxBuf[i][0];
        rxBufDesc[i].bufSize = MAX_FRAME_BUF_SIZE;
        rxBufDesc[i].cbFunc = rxCallback;

        result = adin1110_SubmitRxBuffer(hDevice, &rxBufDesc[i]);
        if (result != ADI_ETH_SUCCESS)
        {
            DEBUG_MESSAGE("Error: Submitting Rx buffer failed.\r\n");
            return result;
        }
    }
    result = adin1110_Enable(hDevice);
    if (result != ADI_ETH_SUCCESS)
    {
        DEBUG_MESSAGE("Error: Enabling device failed.\r\n");
        return result;
    }

    initPQueue(&pQ[0]);

    DEBUG_MESSAGE("ADIN1110 initialization completed successfully.\r\n");
    return result;
}



void LwIP_Init( LwIP_ADIN1110_t* eth,  board_t *boardDetails)
{
    ADIN1110Init(eth);
    lwip_init();
    http_set_ssi_handler(ssiHandler, NULL, 0);
    httpd_init();
    if (boardDetails->ip_addr_fixed == 1)
    {
      ip4_addr_t ip, mask, gw;

      IP4_ADDR(&ip, boardDetails->ip_addr[0], boardDetails->ip_addr[1], boardDetails->ip_addr[2], boardDetails->ip_addr[3]);
      IP4_ADDR(&mask,  boardDetails->net_mask[0], boardDetails->net_mask[1], boardDetails->net_mask[2], boardDetails->net_mask[3]);
      IP4_ADDR(&gw,   boardDetails->gateway[0], boardDetails->gateway[1], boardDetails->gateway[2], boardDetails->gateway[3]);

      netif_add(&eth->netif, &ip, &mask, &gw, eth,
      LwipADIN1110Init, ethernet_input);

      netif_set_default(&eth->netif);
      netif_set_up(&eth->netif);
    }
    else
    {
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
	if ((pQ->nWrQ + 1) % MAX_P_QUEUE == pQ->nRdQ) {
		DEBUG_MESSAGE("Queue overflow: dropping packet of length %d\r\n",
				lenEthFrame);
		return;
	}
	memcpy(&pQ->pData[pQ->nWrQ][0], ethFrame, lenEthFrame);
	pQ->lenData[pQ->nWrQ] = lenEthFrame;
	DEBUG_MESSAGE("Packet enqueued at index %d, length: %d\r\n", pQ->nWrQ,
			lenEthFrame);
	pQ->nWrQ++;
	pQ->nWrQ %= MAX_P_QUEUE;
	DEBUG_MESSAGE("Queue state after enqueue: nWrQ=%d, nRdQ=%d\r\n", pQ->nWrQ,
			pQ->nRdQ);
}

void* readPQ(pQueue_t *pQ) {
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
	DEBUG_MESSAGE("Packet dequeued from index %d, length: %d\r\n", pQ->nRdQ,
			ehtFrmLen);

	pQ->nRdQ++;
	pQ->nRdQ %= MAX_P_QUEUE;

	DEBUG_MESSAGE("Queue state after dequeue: nWrQ=%d, nRdQ=%d\r\n", pQ->nWrQ,
			pQ->nRdQ);

	return (void*) p;
}

uint32_t discoveradin1110(adin1110_DeviceHandle_t hDevice) {
	adi_eth_Result_e result;
	uint32_t                error = 1;

    /****** Driver Init *****/
    for (uint32_t i = 0; i < ADIN1110_INIT_ITER; i++)
    {
        result = adin1110_Init(hDevice, &drvConfig);
        if (result == ADI_ETH_SUCCESS)
        {
            error = 0;
            break;
        }
    }
    return error;
}
#endif
