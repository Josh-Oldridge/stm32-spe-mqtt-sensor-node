/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   RTC Configuration for CN0575 Project
  * @details This file implements RTC configuration for the STM32L496ZG-P Nucleo board in the
  *          CN0575 Single Pair Ethernet (SPE) board project. Initializes the RTC with LSE clock
  *          and provides a time-setting function for SNTP synchronization (via 192.168.1.11),
  *          used by freertos.c’s SensorDataMQTTTask when USE_LWIP is defined. Adjusts for CET/CEST
  *          and logs updates via LPUART1.
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
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
/** @brief Includes for time conversion and logging
  * @details time.h for struct tm and gmtime, stdio.h for printf to LPUART1 in set_system_time.
  */
#include <time.h>
#include <stdio.h>
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
  /**
    * @fn void MX_RTC_Init(void)
	* @brief Initialize RTC for timekeeping
	* @details Configures RTC with LSE clock and initial time/date, called from main.c for SNTP sync.
	*/
  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */
  /**
    * @fn void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
	* @brief MSP initialization for RTC
	* @param [in] rtcHandle RTC handle
	* @details Sets up LSE clock source and enables RTC peripheral for timekeeping in CN0575.
	*/
  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */
  /**
    * @fn void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
	* @brief MSP deinitialization for RTC
	* @param [in] rtcHandle RTC handle
	* @details Disables RTC clock, cleanup for system shutdown or reset in CN0575.
	*/
  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief Check for daylight saving time (DST)
  * @param [in] timeinfo Pointer to struct tm with UTC time
  * @return 1 if DST (CEST) is active, 0 if not (CET)
  * @details Determines if DST applies (March last Sunday 1:00 to October last Sunday 1:00 UTC),
  *          used in set_system_time to adjust for CEST (UTC+2) in CN0575’s time zone (Germany).
  */
static int is_dst(struct tm *timeinfo) {
    int month = timeinfo->tm_mon;
    int day = timeinfo->tm_mday;
    int hour = timeinfo->tm_hour;

    if (month < 2 || month > 9) {
        return 0; // No DST
    }
    if (month > 2 && month < 9) {
        return 1;
    }

    struct tm last_sunday;
    last_sunday = *timeinfo;
    last_sunday.tm_hour = 0;
    last_sunday.tm_min = 0;
    last_sunday.tm_sec = 0;

    last_sunday.tm_mon = 2;
    last_sunday.tm_mday = 31;
    mktime(&last_sunday);
    while (last_sunday.tm_wday != 0) {
        last_sunday.tm_mday--;
        mktime(&last_sunday);
    }
    int dst_start_day = last_sunday.tm_mday;

    last_sunday.tm_mon = 9;
    last_sunday.tm_mday = 31;
    mktime(&last_sunday);
    while (last_sunday.tm_wday != 0) {
        last_sunday.tm_mday--;
        mktime(&last_sunday);
    }
    int dst_end_day = last_sunday.tm_mday;

    if (month == 2) {
        if (day < dst_start_day) return 0;
        if (day > dst_start_day) return 1;
        return (hour >= 1);
    }
    if (month == 9) {
        if (day < dst_end_day) return 1;
        if (day > dst_end_day) return 0;
        return (hour < 1);
    }

    return 0;
}

/**
  * @brief Set system time using RTC
  * @param [in] sec Seconds since epoch (UTC)
  * @param [in] us Microseconds (unused in this implementation)
  * @details Converts SNTP UTC time to CET/CEST, updates RTC, and logs to LPUART1. Called by
  *          freertos.c’s SensorDataMQTTTask via SNTP_SET_SYSTEM_TIME_US for MQTT timestamping.
  */
void set_system_time(uint32_t sec, uint32_t us) {
    time_t rawtime = (time_t)sec;
    struct tm *timeinfo = gmtime(&rawtime);

    timeinfo->tm_hour += 1;

    if (is_dst(timeinfo)) {
        timeinfo->tm_hour += 1;
    }

    if (timeinfo->tm_hour >= 24) {
        timeinfo->tm_hour -= 24;
        timeinfo->tm_mday += 1;
        mktime(timeinfo);
    }

    RTC_TimeTypeDef sTime = {0};
    sTime.Hours = timeinfo->tm_hour;
    sTime.Minutes = timeinfo->tm_min;
    sTime.Seconds = timeinfo->tm_sec;

    RTC_DateTypeDef sDate = {0};
    sDate.WeekDay = timeinfo->tm_wday == 0 ? 7 : timeinfo->tm_wday;
    sDate.Month = timeinfo->tm_mon + 1;
    sDate.Date = timeinfo->tm_mday;
    sDate.Year = timeinfo->tm_year - 100;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    printf("RTC Updated: %02d-%02d-20%02d %02d:%02d:%02d\n",
           sDate.Date, sDate.Month, sDate.Year,
           sTime.Hours, sTime.Minutes, sTime.Seconds);
}


/** @}*/
/* USER CODE END 1 */
