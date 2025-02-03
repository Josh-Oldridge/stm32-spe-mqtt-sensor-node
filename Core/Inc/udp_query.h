#ifndef UDP_QUERY_H
#define UDP_QUERY_H

#include "lwip/err.h"
#include "lwip/ip_addr.h"

typedef enum {
    STATE_IDLE,
    STATE_WAITING_FOR_RESPONSE,
    STATE_WAITING_FOR_RETRY,
    STATE_RESPONSE_RECEIVED
} QueryState_t;

#define MAX_QUERY_RETRIES 5
#define QUERY_TIMEOUT     20000

extern volatile QueryState_t queryState;
extern volatile uint32_t querySentTime;

err_t udp_send_query(void);
void process_udp_query(void);

#endif /* UDP_QUERY_H */
