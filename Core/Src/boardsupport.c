/*
 *---------------------------------------------------------------------------
 *
 * Copyright (c) 2020, 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.
 * and its licensors.By using this software you agree to the terms of the
 * associated Analog Devices Software License Agreement.
 *
 *---------------------------------------------------------------------------
 */

/**
 * @file    boardsupport.c
 * @brief   Board Support Package (BSP) Implementation for STM32L496ZG-P Nucleo Board.
 * @details Implements hardware abstraction for the STM32L496ZG-P in the CN0575 Single Pair
  *          Ethernet (SPE) board project. Manages SPI1 communication with the ADIN1110 MAC-PHY
  *          (despite SPI2 naming), GPIO for LEDs and resets, and LPUART1 debug output. Supports
  *          standalone frame testing or lwIP-based MQTT workflows, integrating with FreeRTOS
  *          tasks using TIM6 as the timebase source. Note: BSP_InitSystem is unused; initialization
  *          occurs in main.c.
 */

/** @addtogroup bsp Board Support Package
 *  @{
 */

#include "boardsupport.h"
#include <string.h>

/** @brief Delay duration for reset assertion (1ms). */
#define RESET_DELAY       (1)

/** @brief Delay duration after reset release (100ms). */
#define AFTER_RESET_DELAY (100)

/** @brief SPI callback data instance for SPI2 (ADIN1110). */
SPI_CallbackData spiCallbackData;

/** @brief SPI handle for Ethernet communication with ADIN1110 (SPI1, not SPI2) */
SPI_HandleTypeDef hEthSpi;

/*
 * @brief Blocking delay function
 * @details Implements a busy-wait delay using HAL_GetTick() (1ms resolution from TIM6), used
 *          for reset timing and lwIP startup in the CN0575 project.
 */
void BSP_delayMs(uint32_t delay)
{
    volatile uint32_t now;
    uint32_t checkTime  = BSP_SysNow();
    while (1)
    {
      now  = BSP_SysNow();
       if (now - checkTime >= delay)
       {
          break;
       }
    }
}

/*
 * @brief Hardware reset to ADIN1110
 * @details Drives Reset_Pin (PD15) low for 1ms, then high with a 100ms delay, used in
 *          HAL_GPIO_EXTI_Callback for reset button handling in the CN0575 project.
 */
void BSP_HWReset(bool set)
{
    DEBUG_MESSAGE("BSP_HWReset: Driving Reset_Pin LOW\r\n");
    HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_RESET);
    BSP_delayMs(RESET_DELAY);

    DEBUG_MESSAGE("BSP_HWReset: Driving Reset_Pin HIGH\r\n");
    HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);
    BSP_delayMs(AFTER_RESET_DELAY);

    DEBUG_MESSAGE("BSP_HWReset: Reset sequence completed\r\n");
}


/* LED functions */

/**
 * @brief Set LED state on a GPIO pin.
 * @details Sets pin low to turn LED on, high to turn off (active-low LEDs).
 */
static void bspLedSet(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, bool on)
{
    if (on)
    {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
    }
}

/**
 * @brief Toggle LED state on a GPIO pin.
 * @details Inverts the current pin state using HAL_GPIO_TogglePin().
 */
static void bspLedToggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
      HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
}

/*
 * @brief Toggle heartbeat LED (LD3 on PB14)
 * @details Toggles LD3 every 250ms in standalone mode or 1ms in lwIP mode via NetworkMaintenanceTask.
 */
void BSP_HeartBeat(void)
{
    bspLedToggle(BSP_LED3_PORT, BSP_LED3_PIN);
}

/*
 * @brief Set heartbeat LED state (orange).
 * @details Controls BSP_LED3_PORT and BSP_LED3_PIN.
 */
void BSP_HeartBeatLed(bool on)
{
    bspLedSet(BSP_LED3_PORT, BSP_LED3_PIN, on);
}

/*
 * @brief Set error LED state (red).
 * @details Controls BSP_LED2_PORT and BSP_LED2_PIN.
 */
void BSP_ErrorLed(bool on)
{
    bspLedSet(BSP_LED2_PORT, BSP_LED2_PIN, on);
}

/*
 * @brief Set functional LED 1 state.
 * @details Controls BSP_LED1_PORT and BSP_LED1_PIN.
 */
void BSP_FuncLed1(bool on)
{
    bspLedSet(BSP_LED1_PORT, BSP_LED1_PIN, on);
}

/*
 * @brief Toggle functional LED 1.
 * @details Toggles BSP_LED1_PORT and BSP_LED1_PIN.
 */
void BSP_FuncLed1Toggle(void)
{
    bspLedToggle(BSP_LED1_PORT, BSP_LED1_PIN);
}

/*
 * Custom function 2 LED
 */
//void BSP_FuncLed2(bool on)
//{
//    bspLedSet(BSP_LED4_PORT, BSP_LED4_PIN, on);
//}
//
//void BSP_FuncLed2Toggle(void)
//{
//    bspLedToggle(BSP_LED4_PORT, BSP_LED4_PIN);
//}

/*
 * @brief Toggle all LEDs.
 * @details Calls MX_Led_Toggle() to invert all LED states.
 */
void BSP_LedToggleAll(void)
{
    MX_Led_Toggle();
}

/*
 * @brief Perform SPI2 write and read operation for ADIN1110 (actually SPI1)
 * @details Delegates to HAL_SPI_Write_Read() with DMA or blocking mode for ADIN1110 communication,
 *          despite SPI2 naming, as SPI1 is used in main.c.
 */
uint32_t BSP_spi2_write_and_read(uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes, bool useDma)
{
    HAL_SPI_Write_Read(pBufferTx,  pBufferRx,  nbBytes, useDma);

    return 0;
}

extern uint32_t HAL_SPI_Register_Callback(ADI_CB const *pfCallback, void *const pCBParam);

/*
 * @brief Register SPI2 callback function for ADIN1110 (actually SPI1)
 * @details Calls HAL_SPI_Register_Callback() to set up SPI1 event handling for ADIN1110 transfers.
 */
uint32_t BSP_spi2_register_callback(ADI_CB const *pfCallback, void *const pCBParam)
{
  HAL_SPI_Register_Callback(pfCallback,  pCBParam);
  return 0;
}

extern uint32_t HAL_INT_N_Register_Callback(ADI_CB const *pfCallback, void *const pCBParam);

/*
 * @brief Register IRQ callback for ADIN1110.
 * @details Delegates to HAL_INT_N_Register_Callback() for interrupt handling.
 */
uint32_t BSP_RegisterIRQCallback(ADI_CB const *intCallback, void * hDevice)
{
  return HAL_INT_N_Register_Callback(intCallback,  hDevice);
}



/*
 * @brief Set SPI2 chip select pin state for ADIN1110.
 * @details Controls ETH_SPI_SS_PIN (active-high) for SPI communication.
 */
void setSPI2Cs(bool set)
{
    if(set == true)
    {
        HAL_GPIO_WritePin(ETH_SPI_SS_GPIO_Port, ETH_SPI_SS_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(ETH_SPI_SS_GPIO_Port, ETH_SPI_SS_Pin, GPIO_PIN_RESET);
    }
}

/*
 * @brief Read configuration pins (CFG0-CFG3).
 * @details Combines states of CFG0-3 pins into a 4-bit value.
 */
void BSP_getConfigPins(uint16_t *value)
{
    uint16_t val16 = 0;
    uint16_t returnVal = 0;

    val16 = HAL_GPIO_ReadPin(CFG0_GPIO_Port, CFG0_Pin);

    returnVal |= val16 << 0;

    val16 = HAL_GPIO_ReadPin(CFG1_GPIO_Port, CFG1_Pin);

    returnVal |= val16 << 1;

    val16 = HAL_GPIO_ReadPin(CFG2_GPIO_Port, CFG2_Pin);

    returnVal |= val16 << 2;

    val16 = HAL_GPIO_ReadPin(CFG3_GPIO_Port, CFG3_Pin);

    returnVal |= val16 << 3;

    *value = returnVal ;
}

/*
 * @brief Write a message to UART.
 * @details Submits string to UART via submitTxBuffer(), returns error if NULL.
 */
uint32_t msgWrite(char * ptr)
{
  uint32_t error = 0;
  if(ptr == NULL)
  {
    error = 1;
  }
  else
  {
    submitTxBuffer ((uint8_t*)ptr, strlen(ptr));
  }
  return error;
}

/*
 * @brief Get the current system tick count
 * @details Returns HAL_GetTick() value in milliseconds (via TIM6), used for heartbeat timing in
 *          standalone mode and delays in both modes of the CN0575 project.
 */
uint32_t BSP_SysNow(void)
{
  return HAL_GetTick();
}

/*
 * @brief Initialize the STM32L496ZG-P system hardware (unused)
 * @details Sets up HAL, GPIO, DMA, and LPUART1; skips clock config (handled in main.c). Not called
 *          in the CN0575 project as initialization occurs directly in main.c.
 */
uint32_t BSP_InitSystem(void)
{
  HAL_StatusTypeDef     result = HAL_OK;

  result = HAL_Init();
  if (result != HAL_OK)
  {
    goto end;
  }
  //SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();


  MX_LPUART1_UART_Init();

end:
  return (uint32_t)result;
}

/** @brief Buffer for debug messages written to UART. */
char aDebugString[200u];

/*
 * @brief Report a failure via UART.
 * @details Outputs "Failed: " prefix, failure reason, and newline to UART.
 */
void common_Fail(char *FailureReason)
{
    char fail[] = "Failed: ";
    char term[] = "\n\r";

    msgWrite(fail);
    msgWrite(FailureReason);
    msgWrite(term);
 }

/*
 * @brief Print performance or debug information via UART.
 * @details Outputs the info string followed by a newline.
 */
void common_Perf(char *InfoString)
{
    char term[] = "\n\r";
    msgWrite(InfoString);
    msgWrite(term);
}

/** @} */
