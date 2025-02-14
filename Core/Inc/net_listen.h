#ifndef NET_LISTEN_H
#define NET_LISTEN_H

#include "lwip/err.h"

#ifdef __cplusplus
extern "C" {
#endif


err_t net_listen_init(void);
void net_listen_process(void);
err_t heartbeat_udp_init(void);
void send_heartbeat(void);


#ifdef __cplusplus
}
#endif

#endif /* NET_LISTEN_H */
