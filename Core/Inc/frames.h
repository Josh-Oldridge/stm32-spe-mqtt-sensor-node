/**
  ******************************************************************************
  * @file    frames.h
  * @brief   Custom Ethernet Frame Definitions for CN0575 Project
  * @details This header defines custom Ethernet frame structures and MAC addresses for the
  *          CN0575 Single Pair Ethernet (SPE) board project on the STM32L496ZG-P Nucleo board.
  *          Used in standalone mode (when USE_LWIP is not defined) to send Layer 2 Ethernet
  *          frames via the ADIN1110 MAC-PHY, bypassing IP-layer functionality. Provides test
  *          frames and multicast MAC addresses for main.c’s frame transmission logic.
  * @addtogroup ethernet Ethernet Utilities
  * @{
  ******************************************************************************
  */

#ifndef FRAMES_H
#define FRAMES_H
#ifndef USE_LWIP
#include <stdint.h>
#include "adi_mac.h"

/** @brief Multicast MAC addresses for standalone frame filtering */
extern uint8_t macAddr[2][6];

/** @brief Predefined test frames for standalone transmission */
extern uint8_t testFrames[TEST_FRAMES_COUNT][MAX_FRAME_SIZE];
#endif /* USE_LWIP */

/**
  * @}
  */

#endif /* FRAMES_H */
