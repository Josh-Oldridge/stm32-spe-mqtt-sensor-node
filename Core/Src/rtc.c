/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
#include <time.h>
#include <stdio.h>
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

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

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
static int is_dst(struct tm *timeinfo) {
    int month = timeinfo->tm_mon;
    int day = timeinfo->tm_mday;
    int hour = timeinfo->tm_hour;

    if (month < 2 || month > 9) { // Before March or after October
        return 0; // No DST
    }
    if (month > 2 && month < 9) { // April to September
        return 1; // DST
    }

    struct tm last_sunday;
    last_sunday = *timeinfo;
    last_sunday.tm_hour = 0;
    last_sunday.tm_min = 0;
    last_sunday.tm_sec = 0;

    // Last Sunday of March
    last_sunday.tm_mon = 2;
    last_sunday.tm_mday = 31;
    mktime(&last_sunday);
    while (last_sunday.tm_wday != 0) {
        last_sunday.tm_mday--;
        mktime(&last_sunday);
    }
    int dst_start_day = last_sunday.tm_mday;

    // Last Sunday of October
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

void set_system_time(uint32_t sec, uint32_t us) {
    time_t rawtime = (time_t)sec;
    struct tm *timeinfo = gmtime(&rawtime);

    // (CET, UTC+1)
    timeinfo->tm_hour += 1;

    // DST in effect --> (CEST, UTC+2)
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

/* USER CODE END 1 */
