#include "net_listen.h"

#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/ip.h"
#include "lwip/pbuf.h"
#include "lwip/inet_chksum.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/udp.h"

#include "main.h"
#include <string.h>

static struct raw_pcb *icmp_raw_pcb = NULL;

static struct udp_pcb *heartbeat_udp_pcb = NULL;

/**
 * @brief  Raw PCB receive callback for ICMP.
 *         If the packet is an ICMP echo request, convert it to an echo reply
 *         and send it back.
 *
 * @param arg Not used.
 * @param pcb The raw PCB.
 * @param p The incoming packet.
 * @param addr The sender's IP address.
 * @param port Not used.
 * @return u8_t 1 if the packet was handled, 0 otherwise.
 */
static u8_t icmp_recv_callback(void *arg, struct raw_pcb *pcb, struct pbuf *p,
                               const ip_addr_t *addr) {
    LWIP_UNUSED_ARG(arg);
    if (p->len >= sizeof(struct icmp_echo_hdr)) {
        struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)p->payload;
        if (ICMPH_TYPE(iecho) == ICMP_ECHO) {
            DEBUG_MESSAGE("[NET_LISTEN] Received ICMP echo request from %s\r\n", ipaddr_ntoa(addr));
            ICMPH_TYPE_SET(iecho, ICMP_ER);
            iecho->chksum = 0;
            iecho->chksum = inet_chksum(iecho, p->len);
            raw_sendto(pcb, p, addr);
            DEBUG_MESSAGE("[NET_LISTEN] Sent ICMP echo reply\r\n");
            return 1;
        }
    }
    return 0;
}

/**
 * @brief  Initializes the network listening module.
 *         Sets up a raw PCB for ICMP and binds it.
 *
 * @retval ERR_OK on success or an error code otherwise.
 */
err_t net_listen_init(void) {
    err_t err;

    icmp_raw_pcb = raw_new(IP_PROTO_ICMP);
    if (icmp_raw_pcb == NULL) {
        DEBUG_MESSAGE("[NET_LISTEN] Failed to create raw PCB for ICMP\r\n");
        return ERR_MEM;
    }
    err = raw_bind(icmp_raw_pcb, IP4_ADDR_ANY);
    if (err != ERR_OK) {
        DEBUG_MESSAGE("[NET_LISTEN] raw_bind failed: %d\r\n", err);
        return err;
    }
    raw_recv(icmp_raw_pcb, icmp_recv_callback, NULL);
    DEBUG_MESSAGE("[NET_LISTEN] ICMP listener initialized\r\n");
    return ERR_OK;
}

/**
 * @brief  Periodic processing for the network listening module.
 *         (For a simple ICMP responder, this may be empty.)
 */
void net_listen_process(void) {

}

err_t heartbeat_udp_init(void) {
	err_t err;
    heartbeat_udp_pcb = udp_new();
    if (heartbeat_udp_pcb == NULL) {
        DEBUG_MESSAGE("[HEARTBEAT] Failed to create UDP PCB for heartbeat\r\n");
        return ERR_MEM;
    }
    ip4_addr_t remoteIP;
    IP4_ADDR(&remoteIP, 192, 168, 1, 11);
    err = udp_connect(heartbeat_udp_pcb, &remoteIP, 5001);
    if (err != ERR_OK) {
        DEBUG_MESSAGE("[HEARTBEAT] UDP connect failed: %d\r\n", err);
        return err;
    }
    return ERR_OK;
}

void send_heartbeat(void) {
    struct pbuf *p;
    const char heartbeatMsg[] = "HEARTBEAT";
    err_t err;

    if (heartbeat_udp_pcb == NULL) {
        DEBUG_MESSAGE("[HEARTBEAT] Heartbeat PCB is not initialized\r\n");
        return;
    }

    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(heartbeatMsg) - 1, PBUF_RAM);
    if (p != NULL) {
        memcpy(p->payload, heartbeatMsg, sizeof(heartbeatMsg) - 1);
        err = udp_send(heartbeat_udp_pcb, p);
        if (err == ERR_OK) {
            DEBUG_MESSAGE("[HEARTBEAT] Sent heartbeat packet\r\n");
        } else {
            DEBUG_MESSAGE("[HEARTBEAT] Failed to send heartbeat: %d\r\n", err);
        }
        pbuf_free(p);
    } else {
        DEBUG_MESSAGE("[HEARTBEAT] Failed to allocate pbuf for heartbeat\r\n");
    }
}
