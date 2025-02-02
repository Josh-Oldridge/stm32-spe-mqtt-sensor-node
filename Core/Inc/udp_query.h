#ifndef UDP_QUERY_H
#define UDP_QUERY_H

#include "lwip/err.h"
#include "lwip/ip_addr.h"

/* The query state machine type */
typedef enum {
    STATE_IDLE,
    STATE_WAITING_FOR_RESPONSE,
    STATE_RESPONSE_RECEIVED
} QueryState_t;

/* The number of query retries and timeout (in milliseconds) */
#define MAX_QUERY_RETRIES 5
#define QUERY_TIMEOUT     20000

/* These variables are visible to other modules (if needed) */
extern volatile QueryState_t queryState;
extern volatile uint32_t querySentTime;

/* These functions are part of the UDP query module.
   (Do not declare the internal static callback here.) */
err_t udp_send_query(void);
void process_udp_query(void);

#endif /* UDP_QUERY_H */
