/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
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
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
  * @file    cc.h
  * @brief   lwIP Architecture-Specific Definitions for CN0575 Project
  * @details This file provides architecture-specific definitions for lwIP on the STM32L496ZG-P
  *          Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project. It configures
  *          data types, byte order, and diagnostic macros for lwIP integration with the ADIN1110
  *          MAC-PHY over SPI1. Used when USE_LWIP is defined, it supports secure MQTT transmission
  *          over TLSv1.2 via FreeRTOS tasks. Key active definitions include DEBUG_MESSAGE for
  *          LPUART1 logging and time-related settings; others (e.g., custom types) are unused or
  *          overridden by stm32l4xx_hal.h.
  * @addtogroup lwip lwIP Configuration
  * @{
  ******************************************************************************
  */

#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__

#include <stdio.h>
#include <stdlib.h>

#include "lwipopts.h"
#include "boardsupport.h"
/*
 * Note: LWIP_TIMEVAL_PRIVATE is set to 0, enabling system <time.h> for struct timeval, used by
 *       lwIP for timing operations in the CN0575 project (e.g., SNTP in freertos.c).
 */
#define LWIP_TIMEVAL_PRIVATE 0
#include <time.h>

/*
 * Note: BYTE_ORDER is set to LITTLE_ENDIAN for STM32L496ZG-P, matching its architecture, used
 *       by lwIP for network byte order conversions in the CN0575 project.
 */
#ifdef PROCESSOR_LITTLE_ENDIAN
  #ifndef BYTE_ORDER
    #define BYTE_ORDER LITTLE_ENDIAN
  #endif
#else
  #ifndef BYTE_ORDER
    #define BYTE_ORDER BIG_ENDIAN
  #endif
#endif

#if 0
/*
 * Note: Custom type definitions (u8_t, s8_t, etc.) are commented out, relying on stm32l4xx_hal.h
 *       types instead, as they are unused in the CN0575 project configuration.
 */
typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   int    u32_t;
typedef signed     int    s32_t;
typedef unsigned   long long    u64_t;
typedef signed     long long    s64_t;
#endif

#define S16_F "d"
#define U16_F "d"
#define S32_F "d"
#define U32_F "x"

#define X16_F "x"
#define X32_F "x"

#define LWIP_RAND rand

#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#define LWIP_PLATFORM_ASSERT(x)

/**
  * @brief Macro to print lwIP diagnostic messages via LPUART1
  * @param[in] x Format string and arguments for DEBUG_MESSAGE
  * @details Uses boardsupport.h’s DEBUG_MESSAGE to output lwIP diagnostics to LPUART1, active in
  *          the CN0575 project for network debugging (e.g., link status in main.c).
  */
#define LWIP_PLATFORM_DIAG(x) do {  DEBUG_MESSAGE x; } while(0)//xil_printf x; } while(0)

/** @} */

#endif /* __ARCH_CC_H__ */
