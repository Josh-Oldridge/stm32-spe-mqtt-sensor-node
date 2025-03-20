/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   RTC Interface for CN0575 Project
  * @details This header declares the RTC initialization and time-setting functions for the
  *          STM32L496ZG-P Nucleo board in the CN0575 Single Pair Ethernet (SPE) board project.
  *          Configures the RTC with LSE clock for timekeeping, used by freertos.c’s
  *          SensorDataMQTTTask to synchronize system time via SNTP (192.168.1.11) when
  *          USE_LWIP is defined, supporting MQTT timestamping.
  * @addtogroup rtc RTC Configuration
  * @{
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */
/**
  * @fn extern RTC_HandleTypeDef hrtc
  * @brief RTC handle for timekeeping
  */
/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
/**
  * @brief Set system time using RTC
  * @param [in] sec Seconds since epoch (UTC)
  * @param [in] us Microseconds (unused in this implementation)
  * @details Updates the RTC with SNTP-provided time, adjusting for CET/CEST, called by freertos.c.
  */
void set_system_time(uint32_t sec, uint32_t us);

/** @}*/
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

