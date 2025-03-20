/**
 * @file
 *
 * lwIP Options Configuration
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/**
  *  Portions Copyright (c) 2020, 2021 Analog Devices, Inc.
  */

/**
  ******************************************************************************
  * @brief   lwIP Configuration Options for CN0575 Project
  * @details Configures lwIP options for the CN0575 Single Pair Ethernet (SPE) board project
  *          on the STM32L496ZG-P Nucleo board with the ADIN1110 MAC-PHY. Enables lightweight
  *          TCP/IP functionality for MQTT over TLSv1.2 via SPI1, integrating with FreeRTOS
  *          tasks (e.g., SensorDataMQTTTask in freertos.c) when USE_LWIP is defined. Includes
  *          DHCP, SNTP, TCP, UDP, and mbedTLS support with minimal memory footprint and no
  *          socket/netconn APIs.
  * @addtogroup lwip lwIP Configuration
  * @{
  ******************************************************************************
  */

#ifndef LWIPOPTS_H_
#define LWIPOPTS_H_

/*
   ----------------------------------
   ---------- System options --------
   ----------------------------------
*/
/** @brief Define processor endianness as little-endian for STM32L496ZG-P
  * @details Matches the STM32L496ZG-P’s architecture, ensuring correct byte order for lwIP’s
  *          network operations (e.g., MQTT packets in client_mqtt.c).
  */
#ifndef PROCESSOR_LITTLE_ENDIAN
#define PROCESSOR_LITTLE_ENDIAN
#endif

/** @brief Disable lwIP system facilities for minimal functionality
  * @details Set to 1 to remove threading and OS features, suitable for CN0575’s single-threaded
  *          FreeRTOS design with NO_SYS operation in freertos.c.
  */
#ifndef NO_SYS
#define NO_SYS                          1
#endif

/** @brief Enable lwIP timers for timeout handling
  * @details Required for managing timeouts (e.g., SNTP retries in freertos.c) despite NO_SYS=1,
  *          using LWIP_TIMERS_CUSTOM is not needed as basic timers suffice.
  */
#define LWIP_TIMERS                     1

/** @brief Disable sys_timeout support when NO_SYS=1
  * @details Set to 0 as NO_SYS=1 eliminates multi-threaded timeout needs; timers are handled
  *          directly by lwIP’s internal mechanisms in the CN0575 project.
  */
#ifndef NO_SYS_NO_TIMERS
#define NO_SYS_NO_TIMERS                0
#endif

/** @brief Disable lightweight protection mechanisms
  * @details Set to 0 as NO_SYS=1 removes need for SYS_ARCH_PROTECT; no interrupt-driven memory
  *          freeing is used in CN0575’s design.
  */
#define SYS_LIGHTWEIGHT_PROT            0

/** @brief Disable socket API
  * @details Set to 0 as CN0575 uses raw lwIP APIs for MQTT (client_mqtt.c) without socket support.
  */
#define LWIP_SOCKET                     0

/** @brief Disable socket compatibility layer
  * @details Set to 0 as no socket API is used, ensuring minimal code footprint in CN0575.
  */
#define LWIP_COMPAT_SOCKETS             0

/** @brief Disable netconn API
  * @details Set to 0 as CN0575 relies on raw lwIP APIs, not the sequential netconn API.
  */
#define LWIP_NETCONN                    0

/** @brief Disable tcpip_timeout for single-threaded operation
  * @details Set to 0 as NO_SYS=1 means no multi-threaded timers are needed; timeouts are handled
  *          within the main loop in freertos.c.
  */
#ifndef LWIP_TCPIP_TIMEOUT
#define LWIP_TCPIP_TIMEOUT              0
#endif

/*
   ------------------------------------
   ---------- Protocol options --------
   ------------------------------------
*/
/** @brief Enable IPv4 support for network communication
  * @details Required for MQTT over TLSv1.2 to the broker at 192.168.1.5 in client_mqtt.c.
  */
#ifndef LWIP_IPV4
#define LWIP_IPV4                       1
#endif

/** @brief Enable ARP for Ethernet frame resolution
  * @details Necessary for Layer 2 communication with ADIN1110 via SPI1, mapping IPs to MACs.
  */
#define LWIP_ARP                        1

/** @brief Set ARP table size to 10 entries
  * @details Sufficient for CN0575’s single MQTT connection and minimal network interactions.
  */
#define ARP_TABLE_SIZE                  10

/** @brief Disable ARP queuing for direct link operation
  * @details Set to 0 as ADIN1110’s direct SPI1 link avoids queuing delays in CN0575.
  */
#define ARP_QUEUEING                    0

/** @brief Enable ICMP for network diagnostics
  * @details Allows ping responses for network testing, useful during CN0575 development.
  */
#ifndef LWIP_ICMP
#define LWIP_ICMP                       1
#endif

/** @brief Set ICMP TTL to maximum for robust diagnostics
  * @details Ensures ICMP packets (e.g., pings) have maximum reach in the network.
  */
#define ICMP_TTL                        255

/** @brief Enable DHCP for dynamic IP assignment
  * @details Obtains IP from the 192.168.1.x network (e.g., 192.168.1.10), used in freertos.c.
  */
#define LWIP_DHCP                       1

/** @brief Enable ARP check during DHCP
  * @details Prevents IP conflicts by verifying availability via ARP, enhancing reliability.
  */
#define DHCP_DOES_ARP_CHECK             1

/** @brief Enable SNTP for time synchronization
  * @details Provides timestamping for MQTT via server at 192.168.1.11, set in freertos.c.
  */
#define LWIP_SNTP 						1

/** @brief Obtain SNTP servers via DHCP
  * @details Uses DHCP-provided server (192.168.1.11 in freertos.c) for time sync.
  */
#define SNTP_GET_SERVERS_FROM_DHCP      1

/** @brief Define SNTP time-setting function
  * @details Links to set_system_time in freertos.c for updating RTC with SNTP data.
  */
#define SNTP_SET_SYSTEM_TIME_US(sec, us) set_system_time(sec, us)

/** @brief Enable TCP for MQTT communication
  * @details Essential for MQTT over TLSv1.2 to the broker at 192.168.1.5 in client_mqtt.c.
  */
#define LWIP_TCP                        1

/** @brief Set TCP TTL to maximum for reliability
  * @details Ensures TCP packets (MQTT messages) have maximum lifespan in the network.
  */
#define TCP_TTL                         255

/** @brief Enable TCP keepalive for persistent connections
  * @details Maintains MQTT connection to 192.168.1.5, preventing drops during idle periods.
  */
#define LWIP_TCP_KEEPALIVE 1

/** @brief Set TCP keepalive idle time to 30 seconds
  * @details Starts probing after 30s of inactivity, balancing connectivity and overhead.
  */
#define TCP_KEEPIDLE                    30000

/** @brief Set TCP keepalive interval to 2 seconds
  * @details Sends probes every 2s after idle, ensuring timely detection of disconnection.
  */
#define TCP_KEEPINTVL                   2000

/** @brief Set TCP keepalive probe count to 6
  * @details Allows 6 probes (12s total) before declaring the MQTT connection dead.
  */
#define TCP_KEEPCNT                     6

/** @brief Enable UDP for optional lightweight communication
  * @details Supports udp_query.c functionality if used, though primarily unused in CN0575.
  */
#define LWIP_UDP                        1

/** @brief Set UDP TTL to maximum for robustness
  * @details Ensures UDP packets have maximum reach, though UDP is minimally used.
  */
#define UDP_TTL                         255

/** @brief Enable ALTCP for TLS over TCP
  * @details Provides abstraction for TLS, required for MQTT security in client_mqtt.c.
  */
#define LWIP_ALTCP                      1

/** @brief Enable TLS support for secure MQTT
  * @details Enables TLSv1.2 for MQTT connection to 192.168.1.5, using certificates.h’s CA cert.
  */
#define LWIP_ALTCP_TLS                  1

/** @brief Use mbedTLS for TLS implementation
  * @details Integrates mbedTLS with broker_ca_cert for secure MQTT in client_mqtt.c.
  */
#define LWIP_ALTCP_TLS_MBEDTLS          1

/*
 ------------------------------------
 ---------- Memory options ----------
 ------------------------------------
 */
/** @brief Use C library malloc/free for memory allocation
  * @details Set to 1 to leverage STM32’s standard malloc/free (via FreeRTOS heap), reducing lwIP’s
  *          internal allocator overhead for MQTT and sensor data in CN0575’s lightweight design.
  */
#ifndef MEM_LIBC_MALLOC
#define MEM_LIBC_MALLOC                 1
#endif

/** @brief Use mem_malloc/mem_free with MEM_LIBC_MALLOC for pool allocation
  * @details Set to 1 to replace lwIP’s pool allocator with mem_malloc/mem_free, simplifying memory
  *          management in CN0575 where FreeRTOS tasks (e.g., SensorDataMQTTTask) handle allocations.
  */
#ifndef MEMP_MEM_MALLOC
#define MEMP_MEM_MALLOC                 1
#endif

/** @brief Set memory alignment to 4 bytes for STM32L496ZG-P
  * @details Matches STM32L496ZG-P’s 32-bit architecture, ensuring efficient memory access for lwIP
  *          buffers (e.g., pbufs in freertos.c’s NetworkMaintenanceTask).
  */
#ifndef MEM_ALIGNMENT
#define MEM_ALIGNMENT                   4
#endif

/** @brief Heap memory size (32 KB) for MQTT and sensor data
  * @details Defines a 32 KB heap, sufficient for MQTT payloads, TLS buffers, and sensor data in
  *          freertos.c, balancing memory use with CN0575’s minimal requirements.
  */
#ifndef MEM_SIZE
#define MEM_SIZE                        (32 * 1024)
#endif

/** @brief Disable separate memory pools (single array used)
  * @details Set to 0 to use a single contiguous array for all pools, reducing complexity as
  *          MEM_LIBC_MALLOC handles allocations in CN0575.
  */
#ifndef MEMP_SEPARATE_POOLS
#define MEMP_SEPARATE_POOLS             0
#endif

/** @brief Disable overflow checking for memory pools
  * @details Set to 0 to skip overflow protection, minimizing overhead in CN0575’s constrained
  *          memory environment where debugging is handled via LPUART1 logs.
  */
#ifndef MEMP_OVERFLOW_CHECK
#define MEMP_OVERFLOW_CHECK             0
#endif

/** @brief Enable sanity checks for memory pool linked lists
  * @details Set to 1 to verify pool integrity after each free operation, enhancing reliability
  *          in CN0575 despite NO_SYS=1, though it adds minor overhead.
  */
#ifndef MEMP_SANITY_CHECK
#define MEMP_SANITY_CHECK               1
#endif

/** @brief Disable custom memory pools using fixed-size arrays
  * @details Set to 0 as CN0575 uses MEM_LIBC_MALLOC instead of lwIP’s pool allocator, avoiding
  *          fixed-size pool complexity.
  */
#ifndef MEM_USE_POOLS
#define MEM_USE_POOLS                   0
#endif

/** @brief Disable trying bigger pools when one is full
  * @details Set to 0 as MEM_USE_POOLS=0 and MEM_LIBC_MALLOC=1 eliminate pool-based allocation,
  *          unnecessary in CN0575’s design.
  */
#ifndef MEM_USE_POOLS_TRY_BIGGER_POOL
#define MEM_USE_POOLS_TRY_BIGGER_POOL   0
#endif

/** @brief Disable custom pool definitions from lwippools.h
  * @details Set to 0 as no custom pools are defined, relying on standard heap allocation in CN0575.
  */
#ifndef MEMP_USE_CUSTOM_POOLS
#define MEMP_USE_CUSTOM_POOLS           0
#endif

/** @brief Disable freeing PBUF_RAM from interrupt context
  * @details Set to 0 as NO_SYS=1 ensures all freeing occurs in the main loop (e.g., NetworkMaintenanceTask),
  *          avoiding ISR complexity in CN0575.
  */
#ifndef LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 0
#endif

/*
 ------------------------------------------------
 ---------- Internal Memory Pool Sizes ----------
 ------------------------------------------------
*/
/** @brief Number of PBUF_ROM/PBUF_REF buffers (4 when DEBUG is defined)
  * @details Set to 4 in debug mode for minimal MQTT use (e.g., referencing static data in client_mqtt.c);
  *          undefined otherwise, relying on default or dynamic allocation via MEM_LIBC_MALLOC.
  */
#ifndef MEMP_NUM_PBUF
#ifdef DEBUG
#define MEMP_NUM_PBUF                   4
#endif
#endif

/** @brief Number of raw connection PCBs (requires LWIP_RAW)
  * @details Set to 0 as LWIP_RAW is disabled; CN0575 uses TCP/UDP for MQTT, not raw IP.
  */
#ifndef MEMP_NUM_RAW_PCB
#define MEMP_NUM_RAW_PCB                0
#endif

/** @brief Number of UDP protocol control blocks
  * @details Set to 3 for potential lightweight UDP use (e.g., udp_query.c if enabled), though unused
  *          in CN0575’s core MQTT-over-TCP design.
  */
#ifndef MEMP_NUM_UDP_PCB
#define MEMP_NUM_UDP_PCB                3
#endif

/** @brief Number of active TCP connections
  * @details Set to 3 to support MQTT connection to 192.168.1.5 plus spares for flexibility in client_mqtt.c.
  */
#ifndef MEMP_NUM_TCP_PCB
#define MEMP_NUM_TCP_PCB                3
#endif

/** @brief Number of listening TCP connections
  * @details Set to 3 to allow potential server-side flexibility, though CN0575 only uses client-side MQTT.
  */
#ifndef MEMP_NUM_TCP_PCB_LISTEN
#define MEMP_NUM_TCP_PCB_LISTEN         3
#endif

/** @brief Number of queued TCP segments for reliability
  * @details Set to 8 to buffer MQTT data segments in client_mqtt.c, ensuring reliable transmission.
  */
#ifndef MEMP_NUM_TCP_SEG
#define MEMP_NUM_TCP_SEG                8
#endif

/** @brief Number of IP packets queued for reassembly (whole packets)
  * @details Set to 0 as IP_REASSEMBLY=0; CN0575 sends full frames via ADIN1110, no reassembly needed.
  */
#ifndef MEMP_NUM_REASSDATA
#define MEMP_NUM_REASSDATA              0
#endif

/** @brief Number of IP fragments simultaneously sent
  * @details Set to 0 as IP_FRAG=0 and ADIN1110’s SPI1 sends full frames, no fragmentation in CN0575.
  */
#ifndef MEMP_NUM_FRAG_PBUF
#define MEMP_NUM_FRAG_PBUF              0
#endif

/** @brief Number of ARP queue entries for outgoing packets
  * @details Set to 2 for minimal buffering during ARP resolution, sufficient for CN0575’s single link.
  */
#ifndef MEMP_NUM_ARP_QUEUE
#define MEMP_NUM_ARP_QUEUE              2
#endif

/** @brief Number of IGMP multicast groups (requires LWIP_IGMP)
  * @details Set to 0 as IGMP is unused; CN0575 uses unicast MQTT communication.
  */
#ifndef MEMP_NUM_IGMP_GROUP
#define MEMP_NUM_IGMP_GROUP             0
#endif

/** @brief Number of system timeouts
  * @details Set to 10 to handle SNTP retries and MQTT keepalives in freertos.c and client_mqtt.c.
  */
#ifndef MEMP_NUM_SYS_TIMEOUT
#define MEMP_NUM_SYS_TIMEOUT            10
#endif

/** @brief Number of netbuf structures (requires netconn API)
  * @details Set to 0 as LWIP_NETCONN=0; CN0575 uses raw APIs, not sequential netbufs.
  */
#ifndef MEMP_NUM_NETBUF
#define MEMP_NUM_NETBUF                 0
#endif

/** @brief Number of netconn structures (requires netconn API)
  * @details Set to 0 as LWIP_NETCONN=0; CN0575 avoids sequential API overhead.
  */
#ifndef MEMP_NUM_NETCONN
#define MEMP_NUM_NETCONN                0
#endif

/** @brief Number of tcpip_msg structures for API callbacks
  * @details Set to 0 as NO_SYS=1 eliminates tcpip_thread; CN0575 uses direct task calls.
  */
#ifndef MEMP_NUM_TCPIP_MSG_API
#define MEMP_NUM_TCPIP_MSG_API          0
#endif

/** @brief Number of tcpip_msg structures for incoming packets
  * @details Set to 0 as NO_SYS=1 avoids tcpip_thread; packet handling is in NetworkMaintenanceTask.
  */
#ifndef MEMP_NUM_TCPIP_MSG_INPKT
#define MEMP_NUM_TCPIP_MSG_INPKT        0
#endif

/** @brief Number of SNMP tree leafs (requires SNMP)
  * @details Set to 0 as SNMP is unused in CN0575’s MQTT-focused design.
  */
#ifndef MEMP_NUM_SNMP_NODE
#define MEMP_NUM_SNMP_NODE              0
#endif

/** @brief Number of SNMP tree branches (requires SNMP)
  * @details Set to 0 as SNMP is not implemented in CN0575.
  */
#ifndef MEMP_NUM_SNMP_ROOTNODE
#define MEMP_NUM_SNMP_ROOTNODE          0
#endif

/** @brief Number of concurrent SNMP requests (requires SNMP)
  * @details Set to 0 as SNMP is disabled; CN0575 uses MQTT instead.
  */
#ifndef MEMP_NUM_SNMP_VARBIND
#define MEMP_NUM_SNMP_VARBIND           0
#endif

/** @brief Number of SNMP OID/values in use (requires SNMP)
  * @details Set to 0 as SNMP is not utilized in CN0575.
  */
#ifndef MEMP_NUM_SNMP_VALUE
#define MEMP_NUM_SNMP_VALUE             0
#endif

/** @brief Number of concurrent lwip_addrinfo calls (requires DNS)
  * @details Set to 0 as DNS lookups are unused; CN0575 uses fixed IPs (e.g., 192.168.1.5).
  */
#ifndef MEMP_NUM_NETDB
#define MEMP_NUM_NETDB                  0
#endif

/** @brief Number of local host list entries (requires dynamic DNS)
  * @details Set to 0 as DNS_LOCAL_HOSTLIST_IS_DYNAMIC is undefined; CN0575 uses static addressing.
  */
#ifndef MEMP_NUM_LOCALHOSTLIST
#define MEMP_NUM_LOCALHOSTLIST          0
#endif

/** @brief Number of active PPPoE interfaces (requires PPPOE_SUPPORT)
  * @details Set to 0 as PPPoE is unused; CN0575 uses Ethernet via ADIN1110.
  */
#ifndef MEMP_NUM_PPPOE_INTERFACES
#define MEMP_NUM_PPPOE_INTERFACES       0
#endif

/** @brief Number of pbuf pool buffers for packet handling
  * @details Set to 4 to support minimal MQTT traffic (e.g., client_mqtt.c payloads) with low memory use.
  */
#ifndef PBUF_POOL_SIZE
#define PBUF_POOL_SIZE                  4
#endif

/** @brief PBUF pool buffer size in bytes
  * @details Set to 1536 to match Ethernet MTU (1500) plus headers (e.g., 40 bytes from PBUF_LINK_HLEN),
  *          accommodating full frames sent via ADIN1110 in freertos.c.
  */
#define PBUF_POOL_BUFSIZE               1536

/*
   ------------------------------------
   ---------- IP options ------------
   ------------------------------------
*/
/** @brief Link header length in bytes
  * @details Set to 40 to account for Ethernet header (14) plus VLAN tagging (4) and padding in SPE,
  *          ensuring proper buffer sizing for ADIN1110 frame handling in freertos.c.
  */
#define PBUF_LINK_HLEN                  40

/** @brief Disable IP options in packets
  * @details Set to 0 as MQTT in client_mqtt.c doesn’t require IP options, reducing overhead.
  */
#define IP_OPTIONS                      0

/** @brief Disable IP forwarding
  * @details Set to 0 as CN0575 operates as a single device, not a router, in the 192.168.1.x network.
  */
#define IP_FORWARD                      0

/** @brief Disable IP reassembly
  * @details Set to 0 since ADIN1110 sends full frames via SPI1, avoiding fragmentation in CN0575.
  */
#define IP_REASSEMBLY                   0

/** @brief Disable IP fragmentation
  * @details Set to 0 as full Ethernet frames are sent via ADIN1110, no need for IP-level splitting.
  */
#define IP_FRAG                         0

/** @brief Maximum number of pbufs for IP reassembly
  * @details Set to 0 as IP_REASSEMBLY=0; no reassembly buffers needed in CN0575.
  */
#define IP_REASS_MAX_PBUFS              0

/** @brief Maximum MTU for IP fragmentation
  * @details Set to 1500 (Ethernet standard), though unused as IP_FRAG=0 in CN0575.
  */
#define IP_FRAG_MAX_MTU                 1500

/** @brief Set default IP TTL to maximum
  * @details Ensures IP packets (e.g., MQTT) have maximum lifespan, set to 255 for robustness.
  */
#define IP_DEFAULT_TTL                  255

/*
   ------------------------------------
   ---------- TCP options -----------
   ------------------------------------
*/
/** @brief TCP Maximum Segment Size in bytes
  * @details Set to 1460 to optimize MQTT payload size in client_mqtt.c, balancing efficiency and MTU.
  */
#ifndef TCP_MSS
#define TCP_MSS                         1460
#endif

/** @brief TCP sender buffer space in bytes
  * @details Set to 4096 to accommodate MQTT payloads and TLS overhead in client_mqtt.c.
  */
#ifndef TCP_SND_BUF
#define TCP_SND_BUF                     4096
#endif

/** @brief TCP window size in bytes
  * @details Set to 4096 (≥ 2 * TCP_MSS) for efficient MQTT data flow in client_mqtt.c.
  */
#ifndef TCP_WND
#define TCP_WND                         4096
#endif

/** @brief Maximum number of TCP data retransmissions
  * @details Set to 12 for robust MQTT delivery, allowing retries over unreliable links.
  */
#ifndef TCP_MAXRTX
#define TCP_MAXRTX                      12
#endif

/** @brief Maximum number of TCP SYN retransmissions
  * @details Set to 6 to retry connection establishment to 192.168.1.5, balancing reliability and speed.
  */
#ifndef TCP_SYNMAXRTX
#define TCP_SYNMAXRTX                   6
#endif

/** @brief Enable queuing of out-of-sequence TCP segments
  * @details Set to LWIP_TCP (1) to buffer segments for MQTT reliability, using minimal memory in CN0575.
  */
#ifndef TCP_QUEUE_OOSEQ
#define TCP_QUEUE_OOSEQ                 (LWIP_TCP)
#endif

/** @brief TCP sender buffer space in pbufs
  * @details Calculated as (4 * TCP_SND_BUF + (TCP_MSS - 1)) / TCP_MSS, ensuring sufficient queue depth.
  */
#ifndef TCP_SND_QUEUELEN
#define TCP_SND_QUEUELEN                ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#endif

/** @brief TCP oversize segment option
  * @details Set to TCP_MSS (1460) to allow efficient segment sizing for MQTT in client_mqtt.c.
  */
#ifndef TCP_OVERSIZE
#define TCP_OVERSIZE                    TCP_MSS
#endif

/** @brief TCP retransmission timeout in milliseconds
  * @details Set to 3000 ms for initial retransmission delay, balancing responsiveness and stability.
  */
#define TCP_RTO                         3000

/** @brief Enable TCP Selective ACK (SACK) for retransmission
  * @details Set to 1 to improve MQTT retransmission efficiency over potentially lossy links.
  */
#define LWIP_TCP_SACK_OUT               1

/*
   ------------------------------------
   ---------- RAW options -----------
   ------------------------------------
*/
/** @brief Disable RAW IP API
  * @details Set to 0 as CN0575 uses TCP for MQTT, not raw IP, minimizing unused features.
  */
#ifndef LWIP_RAW
#define LWIP_RAW                        0
#endif

/*
   ------------------------------------
   ---------- Statistics options ----
   ------------------------------------
*/
/** @brief Disable lwIP statistics collection
  * @details Set to 0 to reduce memory/CPU overhead; CN0575 relies on LPUART1 logs for diagnostics.
  */
#define LWIP_STATS                      0

/** @brief Disable memory pool statistics
  * @details Set to 0 as MEMP_STATS requires LWIP_STATS=1; unused in CN0575’s minimal setup.
  */
#define MEMP_STATS                      0

/*
   ------------------------------------
   ---------- Checksum options ------
   ------------------------------------
*/
/** @brief Select checksum algorithm (3 for hardware assist)
  * @details Set to 3 to leverage STM32’s hardware checksum offload, optimizing performance.
  */
#define LWIP_CHKSUM_ALGORITHM           3

/** @brief Enable TCP checksum generation
  * @details Set to 1 to ensure MQTT packet integrity, offloaded to hardware in CN0575.
  */
#define CHECKSUM_GEN_TCP 	            1

/** @brief Enable UDP checksum generation
  * @details Set to 1 for potential UDP use (e.g., udp_query.c), though minimal in CN0575.
  */
#define CHECKSUM_GEN_UDP 	            1

/** @brief Enable IP checksum generation
  * @details Set to 1 for IP packet integrity, offloaded to STM32 hardware.
  */
#define CHECKSUM_GEN_IP  	            1

/** @brief Enable TCP checksum verification
  * @details Set to 1 to validate incoming MQTT packets, using hardware assist.
  */
#define CHECKSUM_CHECK_TCP              1

/** @brief Enable UDP checksum verification
  * @details Set to 1 for optional UDP packets, though rarely used in CN0575.
  */
#define CHECKSUM_CHECK_UDP              1

/** @brief Enable IP checksum verification
  * @details Set to 1 to check incoming IP packets, leveraging STM32 hardware.
  */
#define CHECKSUM_CHECK_IP 	            1

/** @brief Enable ICMP checksum generation
  * @details Set to 1 for ping responses, offloaded to hardware.
  */
#define CHECKSUM_GEN_ICMP               1

/** @brief Enable ICMP checksum verification
  * @details Set to 1 to validate incoming pings, using hardware assist.
  */
#define CHECKSUM_CHECK_ICMP             1

/*
 ---------------------------------
 ---------- DEBUG options ----------
 ---------------------------------
 */
/** @brief Define all debug levels (unused as debugging is off)
  * @details Set to 0x00; irrelevant as LWIP_DEBUG is disabled in CN0575.
  */
#define LWIP_DBG_LEVEL_ALL              0x00

/** @brief Disable all lwIP debugging
  * @details Set to LWIP_DBG_OFF; CN0575 uses LPUART1 logs (e.g., MQTT_CLIENT_DEBUG) instead.
  */
#define LWIP_DEBUG                      LWIP_DBG_OFF

/** @brief Disable timers debug output
  * @details Set to LWIP_DBG_OFF; no timer logging needed in production.
  */
#define TIMERS_DEBUG                    LWIP_DBG_OFF

/** @brief Disable ICMP debug output
  * @details Set to LWIP_DBG_OFF; ping diagnostics handled externally.
  */
#define ICMP_DEBUG                      LWIP_DBG_OFF

/** @brief Disable netif debug output
  * @details Set to LWIP_DBG_OFF; network interface logs not required.
  */
#define NETIF_DEBUG                     LWIP_DBG_OFF

/** @brief Disable IP debug output
  * @details Set to LWIP_DBG_OFF; IP operations logged via LPUART1 if needed.
  */
#define IP_DEBUG                        LWIP_DBG_OFF

/** @brief Disable ARP debug output
  * @details Set to LWIP_DBG_OFF; ARP handled silently by ADIN1110 link.
  */
#define ETHARP_DEBUG                    LWIP_DBG_OFF

/** @brief Disable MQTT debug output
  * @details Set to LWIP_DBG_OFF; MQTT debugging uses MQTT_CLIENT_DEBUG in freertos.c.
  */
#define MQTT_DEBUG                      LWIP_DBG_OFF

/** @brief Disable TCP debug output
  * @details Set to LWIP_DBG_OFF; TCP logs managed via LPUART1 in client_mqtt.c.
  */
#define TCP_DEBUG                       LWIP_DBG_OFF

/** @brief Disable UDP debug output
  * @details Set to LWIP_DBG_OFF; minimal UDP use doesn’t require logs.
  */
#define UDP_DEBUG                       LWIP_DBG_OFF

/** @brief Disable memory debug output
  * @details Set to LWIP_DBG_OFF; memory issues tracked via LPUART1 if needed.
  */
#define MEM_DEBUG                       LWIP_DBG_OFF

/** @brief Disable pbuf debug output
  * @details Set to LWIP_DBG_OFF; pbuf operations logged externally if necessary.
  */
#define PBUF_DEBUG                      LWIP_DBG_OFF

/*
   ------------------------------------
   ---------- Misc options ----------
   ------------------------------------
*/
/** @brief Enable netif hostname support
  * @details Set to 1 to advertise "STM32_Client" as hostname in DHCP/MQTT, per client_mqtt.c.
  */
#ifndef LWIP_NETIF_HOSTNAME
#define LWIP_NETIF_HOSTNAME             1
#endif

/** @brief Enable link speed auto-detection
  * @details Set to 1 to auto-negotiate speed with ADIN1110, ensuring compatibility in CN0575.
  */
#define CONFIG_LINKSPEED_AUTODETECT     1

/** @brief Number of parallel HTTPD connections (unused in CN0575)
  * @details Set to 0 as HTTP server functionality is not implemented; CN0575 uses MQTT only.
  */
#define MEMP_NUM_PARALLEL_HTTPD_CONNS   0

/** @brief Number of parallel HTTPD SSI connections (unused in CN0575)
  * @details Set to 0 as Server-Side Includes are not used; CN0575 focuses on MQTT.
  */
#define MEMP_NUM_PARALLEL_HTTPD_SSI_CONNS 0
#endif

/** @}*/
