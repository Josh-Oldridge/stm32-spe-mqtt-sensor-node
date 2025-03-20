/**
  ******************************************************************************
  * @file    udp_query.h
  * @brief   UDP Query Interface for CN0575 Project (Unused)
  * @details This header defines the UDP query interface for the CN0575 Single Pair Ethernet
  *          (SPE) board project on the STM32L496ZG-P Nucleo board. Originally designed to send
  *          UDP packets to 192.168.1.11 every second for testing, ensuring ICMP ping responses
  *          by keeping ADIN1110 buffers active. Currently unused in favor of interrupt-driven
  *          ADIN1110 handling in freertos.c when USE_LWIP is defined.
  * @addtogroup network Network Utilities
  * @{
  ******************************************************************************
  */

#ifndef UDP_QUERY_H
#define UDP_QUERY_H

#include "lwip/err.h"
#include "lwip/ip_addr.h"

/** @brief State machine states for UDP query process
  * @details Enumerates states for tracking UDP query send/receive cycles, used in process_udp_query.
  */
typedef enum {
    STATE_IDLE,              /*!< No query active */
    STATE_WAITING_FOR_RESPONSE, /*!< Awaiting UDP response */
    STATE_WAITING_FOR_RETRY, /*!< Awaiting retry after timeout (unused state) */
    STATE_RESPONSE_RECEIVED  /*!< Response received, query complete */
} QueryState_t;

/** @brief Maximum number of UDP query retries
  * @details Set to 5 attempts before giving up on a query response.
  */
#define MAX_QUERY_RETRIES 5

/** @brief Timeout duration for UDP query response in milliseconds
  * @details Set to 5000 ms (5s) to wait for a response before retrying.
  */
#define QUERY_TIMEOUT     5000

/** @brief Current state of the UDP query process
  * @details Tracks the state machine state, volatile as it’s updated across tasks/callbacks.
  */
extern volatile QueryState_t queryState;

/** @brief Timestamp of the last UDP query sent
  * @details Stores the time (in ms) of the last query, volatile for callback/task access.
  */
extern volatile uint32_t querySentTime;

/**
  * @brief Send a UDP query packet
  * @return ERR_OK on success, lwIP error code on failure
  * @details Sends a UDP query ("CMD:QUERY:LD1_ON?") to 192.168.1.11:5000, originally for testing.
  */
err_t udp_send_query(void);

/**
  * @brief Process the UDP query state machine
  * @details Manages UDP query retries and state transitions, originally run every second for testing.
  */
void process_udp_query(void);

#endif /* UDP_QUERY_H */

/**
  * @}
  */
