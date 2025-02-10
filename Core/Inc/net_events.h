#ifndef NET_EVENTS_H
#define NET_EVENTS_H

#include "FreeRTOS.h"
#include "event_groups.h"

extern EventGroupHandle_t netEventGroup;

#define NET_EVENT_DHCP_CONFIGURED  (1 << 0)

#endif /* NET_EVENTS_H */
