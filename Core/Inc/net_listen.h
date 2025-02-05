#ifndef NET_LISTEN_H
#define NET_LISTEN_H

#include "lwip/err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initializes the network listening module.
 *         This sets up a raw PCB to listen for ICMP (ping) packets and
 *         automatically sends an echo reply when a ping is received.
 * @retval ERR_OK if the initialization is successful.
 */
err_t net_listen_init(void);

/**
 * @brief  Optionally, process any periodic tasks related to network listening.
 *         For a simple ICMP responder using a raw PCB, no periodic processing may be needed.
 */
void net_listen_process(void);

err_t heartbeat_udp_init(void);

/* Send a heartbeat packet */
void send_heartbeat(void);


#ifdef __cplusplus
}
#endif

#endif /* NET_LISTEN_H */
