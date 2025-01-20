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

#include "boardsupport.h"
#include "main.h"
#include "stm32l4xx_hal_spi.h"
#include "hal.h"
#include "bsp_config.h"
#include <string.h>

#define RESET_DELAY       (1)
#define AFTER_RESET_DELAY (100)


SPI_CallbackData spiCallbackData;

SPI_HandleTypeDef hEthSpi;

void ETH_SPI_Init(void) {
    hEthSpi.Instance = SPI1;
    hEthSpi.Init.Mode = SPI_MODE_MASTER;
    hEthSpi.Init.Direction = SPI_DIRECTION_2LINES;
    hEthSpi.Init.DataSize = SPI_DATASIZE_8BIT;
    hEthSpi.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hEthSpi.Init.CLKPhase = SPI_PHASE_2EDGE;
    hEthSpi.Init.NSS = SPI_NSS_SOFT;
    hEthSpi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hEthSpi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hEthSpi.Init.TIMode = SPI_TIMODE_DISABLE;
    hEthSpi.Init.CRCCalculation = SPI_CRCCALCULATION_ENABLE;
    hEthSpi.Init.CRCPolynomial = 7;
    hEthSpi.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hEthSpi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

    if (HAL_SPI_Init(&hEthSpi) != HAL_OK) {
        Error_Handler();
    }
}

void ADIN1110_CS_Select(void);
void ADIN1110_CS_Deselect(void);

/*
 * @brief Blocking delay function
 *
 * @param [in]      delay - delay in miliseconds
 * @param [out]     none
 * @return none
 *
 * @details Based on assumption that SysTick counter fires every milisecond
 *
 * @sa
 */
void BSP_delayMs(uint32_t delay)
{
    volatile uint32_t now;
    uint32_t checkTime  = BSP_SysNow();
    /* Read SysTick Timer every Ms*/
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
 * @brief Hardware reset to DUT
 *
 * @param [in]      set - obosolete, keep it for consistancy with older versions
 * @param [out]     none
 * @return none
 *
 * @details Puld down Reset Pin, wait for 1mS release the Reset Pin
 *
 * @sa
 */
void BSP_HWReset(bool set)
{
    DEBUG_MESSAGE("BSP_HWReset: Driving Reset_Pin LOW\r\n");
    HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_RESET);
    BSP_delayMs(2000);

    DEBUG_MESSAGE("BSP_HWReset: Driving Reset_Pin HIGH\r\n");
    HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);
    BSP_delayMs(500);

    DEBUG_MESSAGE("BSP_HWReset: Reset sequence completed\r\n");
}


/* LED functions */

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

static void bspLedToggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
      HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
}

/*
 * Heartbeat LED, ORANGE
 */
void BSP_HeartBeat(void)
{
    bspLedToggle(BSP_LED3_PORT, BSP_LED3_PIN);
}

/*
 * HeartBeat LED, ORANGE
 */
void BSP_HeartBeatLed(bool on)
{
    bspLedSet(BSP_LED3_PORT, BSP_LED3_PIN, on);
}

/*
 * Error LED, RED
 */
void BSP_ErrorLed(bool on)
{
    bspLedSet(BSP_LED2_PORT, BSP_LED2_PIN, on);
}

/*
 * Custom function 1 LED
 */
void BSP_FuncLed1(bool on)
{
    bspLedSet(BSP_LED1_PORT, BSP_LED1_PIN, on);
}

void BSP_FuncLed1Toggle(void)
{
    bspLedToggle(BSP_LED1_PORT, BSP_LED1_PIN);
}

/*
 * Custom function 2 LED
 */
void BSP_FuncLed2(bool on)
{
    bspLedSet(BSP_LED4_PORT, BSP_LED4_PIN, on);
}

void BSP_FuncLed2Toggle(void)
{
    bspLedToggle(BSP_LED4_PORT, BSP_LED4_PIN);
}

/* All LEDs toggle, used to indicate hardware failure on the board */
void BSP_LedToggleAll(void)
{
    MX_Led_Toggle();
}

uint32_t BSP_spi2_write_and_read(uint8_t *pBufferTx, uint8_t *pBufferRx, uint32_t nbBytes, bool useDma)
{
    ADIN1110_CS_Select();
    HAL_SPI_TransmitReceive_DMA(&hspi1, pBufferTx, pBufferRx, nbBytes);
    return 0;
}

/*at
 * @brief Helper for Access BSP EEPROM, chip select is active high
 *
 * @param [in]      output - true/false
 * @param [out]     none
 * @return pin value
 *
 * @details
 *
 * @sa
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


/**
  * @brief  Return the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *   This parameter should be: BUTTON_USER
  * @retval Button state.
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

uint32_t BSP_SysNow(void)
{
  return HAL_GetTick();
}


uint32_t BSP_InitSystem(void)
{
  HAL_StatusTypeDef     result = HAL_OK;

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  result = HAL_Init();
  if (result != HAL_OK)
  {
    goto end;
  }

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();


  MX_LPUART1_UART_Init();

end:
  return (uint32_t)result;
}

char aDebugString[150u];

void common_Fail(char *FailureReason)
{
    char fail[] = "Failed: ";
    char term[] = "\n\r";

    /* Ignore return codes since there's nothing we can do if it fails */
    msgWrite(fail);
    msgWrite(FailureReason);
    msgWrite(term);
 }

void common_Perf(char *InfoString)
{
    char term[] = "\n\r";

    /* Ignore return codes since there's nothing we can do if it fails */
    msgWrite(InfoString);
    msgWrite(term);
}

uint32_t BSP_RegisterIRQCallback(ADI_CB const *intCallback, void *hDevice) {
    if (intCallback != NULL) {
        HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
        return 0;
    }
    return 1;
}

uint32_t BSP_spi2_register_callback(ADI_CB pfCallback, void *const pCBParam) {
    if (pfCallback != NULL) {
    	spiCallbackData.callback = pfCallback;
        spiCallbackData.userData = pCBParam;
        return 0;
    }
    return 1;
}

void ADIN1110_CS_Select(void)
{
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
}

void ADIN1110_CS_Deselect(void)
{
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
}


