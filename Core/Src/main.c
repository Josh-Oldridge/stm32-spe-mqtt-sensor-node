/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adin1110.h"
#include "boardsupport.h"
#include "frames.h"
#ifdef USE_LWIP
#include "lwIP_adin1110_app.h"
#include "lwip/timeouts.h"
#endif /* USE_LWIP */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


adin1110_DeviceStruct_t dev;
adin1110_DeviceHandle_t hDevice = &dev;

#ifndef USE_LWIP
uint8_t devMem[ADIN1110_DEVICE_SIZE];

adin1110_DriverConfig_t drvConfig = { .pDevMem = (void*) devMem, .devMemSize =
		sizeof(devMem), .fcsCheckEn = false, };


void printStats(adin1110_DeviceHandle_t hDevice);

uint8_t dest_mac[6] = { 0xAC, 0x1A, 0x3D, 0xAC, 0xD0, 0x33 };  // Dell MAC
uint8_t mySourceMac[6] = { 0x00, 0xE0, 0x22, 0xFE, 0xDA, 0xCA };
uint16_t myEtherType = 0x88B5;
const char payloadStr[] = "CMD:RUN|notepad.exe|Hello from ADIN1110!";
uint32_t payloadLen = strlen(payloadStr);

uint32_t txIdx = 0;
volatile uint32_t rxIdx = 0;
volatile uint32_t expectedTxIdx;
volatile uint32_t expectedRxIdx;
volatile uint32_t errorTxIdx;
volatile uint32_t errorRxIdx;

static uint8_t rxBuf[BUFF_DESC_COUNT][MAX_FRAME_BUF_SIZE] __attribute__((aligned(4)));
static uint8_t txBuf[BUFF_DESC_COUNT][MAX_FRAME_BUF_SIZE] __attribute__((aligned(4)));

bool txBufAvailable[BUFF_DESC_COUNT];

#else /* USE_LWIP defined */
board_t boardDetails;
LwIP_ADIN1110_t myConn;

#endif  /* USE_LWIP */

#ifndef USE_LWIP
static void txCallback(void *pCBParam, uint32_t Event, void *pArg) {
	adi_eth_BufDesc_t *pTxBufDesc = (adi_eth_BufDesc_t*) pArg;
	uint32_t idx;

	txIdx++;
	memcpy(&idx, &pTxBufDesc->pBuf[14], 4);

	if (idx != expectedTxIdx) {
		errorTxIdx++;
	}
	expectedTxIdx = idx + 1;
	for (uint32_t i = 0; i < BUFF_DESC_COUNT; i++) {
		if (&txBuf[i][0] == pTxBufDesc->pBuf) {
			txBufAvailable[i] = true;
			break;
		}
	}
}

static void rxCallback(void *pCBParam, uint32_t Event, void *pArg) {
	adin1110_DeviceHandle_t hDevice = (adin1110_DeviceHandle_t) pCBParam;
	adi_eth_BufDesc_t *pRxBufDesc = (adi_eth_BufDesc_t*) pArg;
	uint32_t idx;

	rxIdx++;
	memcpy(&idx, &pRxBufDesc->pBuf[14], 4);

	if (idx != expectedRxIdx) {
		errorRxIdx++;
	}
	expectedRxIdx = idx + 1;

	adin1110_SubmitRxBuffer(hDevice, pRxBufDesc);
}

void cbLinkChange(void *pCBParam, uint32_t Event, void *pArg) {
	adi_eth_LinkStatus_e linkStatus;
	linkStatus = *(adi_eth_LinkStatus_e*) pArg;
	if (linkStatus == ADI_ETH_LINK_STATUS_UP) {
		DEBUG_MESSAGE("Ethernet Link Status: UP\r");
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET); // Turn on LD3
	} else {
		DEBUG_MESSAGE("Ethernet Link Status: DOWN\r");
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET); // Turn off LD3
	}
}
#endif  /* USE_LWIP */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifndef USE_LWIP
void printStats(adin1110_DeviceHandle_t hDevice) {
	adi_eth_Result_e result = ADI_ETH_SUCCESS;
	adi_eth_MacStatCounters_t stats;
	bool fail;

	result = adin1110_GetStatCounters(hDevice, &stats);
	DEBUG_RESULT("adin1110_GetStatCounters", result, ADI_ETH_SUCCESS);

	fail = false;
	fail = fail || (errorTxIdx || errorRxIdx);
	fail = fail || (stats.RX_CRC_ERR_CNT > 0);
	fail = fail || (stats.RX_ALGN_ERR_CNT > 0);
	fail = fail || (stats.RX_LS_ERR_CNT > 0);
	fail = fail || (stats.RX_PHY_ERR_CNT > 0);
	fail = fail || (stats.RX_DROP_FULL_CNT > 0);
	fail = fail || (stats.RX_DROP_FILT_CNT > 0);

	if (fail) {
		DEBUG_MESSAGE("Result: FAIL\r");
		DEBUG_MESSAGE("    Tx index errors: %" PRIu32 "\r", errorTxIdx);
		DEBUG_MESSAGE("    Rx index errors: %" PRIu32 "\r", errorRxIdx);
		BSP_ErrorLed(true);
	} else {
		DEBUG_MESSAGE("Result: PASS\r");
	}
	DEBUG_MESSAGE("Summary:\r");
	DEBUG_MESSAGE("     Sent frames:        %" PRIu32 "\r", txIdx);
	DEBUG_MESSAGE("     Received frames:    %" PRIu32 "\r", rxIdx);
	DEBUG_MESSAGE("     Statistics counters:\r");
	DEBUG_MESSAGE("         TX_FRM_CNT         = %" PRIu32 "\r",
			stats.TX_FRM_CNT);
	DEBUG_MESSAGE("         TX_UCAST_CNT       = %" PRIu32 "\r",
			stats.TX_UCAST_CNT);
	DEBUG_MESSAGE("         TX_MCAST_CNT       = %" PRIu32 "\r",
			stats.TX_MCAST_CNT);
	DEBUG_MESSAGE("         TX_BCAST_CNT       = %" PRIu32 "\r",
			stats.TX_BCAST_CNT);
	DEBUG_MESSAGE("         RX_FRM_CNT         = %" PRIu32 "\r",
			stats.RX_FRM_CNT);
	DEBUG_MESSAGE("         RX_UCAST_CNT       = %" PRIu32 "\r",
			stats.RX_UCAST_CNT);
	DEBUG_MESSAGE("         RX_MCAST_CNT       = %" PRIu32 "\r",
			stats.RX_MCAST_CNT);
	DEBUG_MESSAGE("         RX_BCAST_CNT       = %" PRIu32 "\r",
			stats.RX_BCAST_CNT);
	DEBUG_MESSAGE("         RX_CRC_ERR_CNT     = %" PRIu32 "\r",
			stats.RX_CRC_ERR_CNT);
	DEBUG_MESSAGE("         RX_ALGN_ERR_CNT    = %" PRIu32 "\r",
			stats.RX_ALGN_ERR_CNT);
	DEBUG_MESSAGE("         RX_LS_ERR_CNT      = %" PRIu32 "\r",
			stats.RX_LS_ERR_CNT);
	DEBUG_MESSAGE("         RX_PHY_ERR_CNT     = %" PRIu32 "\r",
			stats.RX_PHY_ERR_CNT);
	DEBUG_MESSAGE("         RX_DROP_FULL_CNT   = %" PRIu32 "\r",
			stats.RX_DROP_FULL_CNT);
	DEBUG_MESSAGE("         RX_DROP_FILT_CNT   = %" PRIu32 "\r",
			stats.RX_DROP_FILT_CNT);
}
#endif  /* USE_LWIP */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_LPUART1_UART_Init();
  /* USER CODE BEGIN 2 */
	#ifndef USE_LWIP
	adi_eth_Result_e result;
	adin1110_DeviceStruct_t deviceStruct;
	adin1110_DeviceHandle_t hDevice = &deviceStruct;

	#else /* USE_LWIP defined */
	boardDetails.mac[0] = 0x00;
	boardDetails.mac[1] = 0xE0;
	boardDetails.mac[2] = 0x22;
	boardDetails.mac[3] = 0xFE;
	boardDetails.mac[4] = 0xDA;
	boardDetails.mac[5] = 0xCA;
	boardDetails.ip_addr[0] = 192;
	boardDetails.ip_addr[1] = 168;
	boardDetails.ip_addr[2] = 1;
	boardDetails.ip_addr[3] = 100;
	boardDetails.net_mask[0] = 255;
	boardDetails.net_mask[1] = 255;
	boardDetails.net_mask[2] = 255;
	boardDetails.net_mask[3] = 0;
	boardDetails.gateway[0] = 192;
	boardDetails.gateway[1] = 168;
	boardDetails.gateway[2] = 1;
	boardDetails.gateway[3] = 1;
	boardDetails.ip_addr_fixed = IP_DYNAMIC;
	#endif  /* USE_LWIP */

	#ifndef USE_LWIP
	for (uint32_t i = 0; i < ADIN1110_INIT_ITER; i++) {
		result = adin1110_Init(hDevice, &drvConfig);
		if (result == ADI_ETH_SUCCESS) {
			break;
		}
	}

	DEBUG_RESULT("No MACPHY device found", result, ADI_ETH_SUCCESS);

	result = adin1110_AddAddressFilter(hDevice, &macAddr[0][0], NULL, 0);
	DEBUG_RESULT("adin1110_AddAddressFilter", result, ADI_ETH_SUCCESS);

	result = adin1110_AddAddressFilter(hDevice, &macAddr[1][0], NULL, 0);
	DEBUG_RESULT("adin1110_AddAddressFilter", result, ADI_ETH_SUCCESS);

	result = adin1110_SyncConfig(hDevice);
	DEBUG_RESULT("adin1110_SyncConfig", result, ADI_ETH_SUCCESS);

	result = adin1110_RegisterCallback(hDevice, cbLinkChange,
			ADI_MAC_EVT_LINK_CHANGE);
	DEBUG_RESULT("adin1110_RegisterCallback (ADI_MAC_EVT_LINK_CHANGE)", result,
			ADI_ETH_SUCCESS);

	adi_eth_BufDesc_t rxBufDesc[BUFF_DESC_COUNT];
	adi_eth_BufDesc_t txBufDesc[BUFF_DESC_COUNT];
	uint32_t txBufDescIdx = 0;
	adi_eth_LinkStatus_e linkStatus;
	uint32_t frameIdx = 0;
	uint32_t heartbeatTicks = 0;

	for (uint32_t i = 0; i < BUFF_DESC_COUNT; i++) {
		memcpy(&txBuf[i], &testFrames[i % 2][0], MAX_FRAME_SIZE);
		txBufAvailable[i] = true;

		rxBufDesc[i].pBuf = &rxBuf[i][0];
		rxBufDesc[i].bufSize = MAX_FRAME_BUF_SIZE;
		rxBufDesc[i].cbFunc = rxCallback;
		result = adin1110_SubmitRxBuffer(hDevice, &rxBufDesc[i]);
	}

	result = adin1110_Enable(hDevice);
	DEBUG_RESULT("Device enable error", result, ADI_ETH_SUCCESS);

	do {
		result = adin1110_GetLinkStatus(hDevice, &linkStatus);
		DEBUG_RESULT("adin1110_GetLinkStatus", result, ADI_ETH_SUCCESS);
	} while (linkStatus != ADI_ETH_LINK_STATUS_UP);
	HAL_Delay(5000);

	expectedRxIdx = 0;
	expectedTxIdx = 0;
	errorTxIdx = 0;
	errorRxIdx = 0;
	#else /* USE_LWIP defined */

	/* Initialize lwIP stack */
	uint32_t result = discoveradin1110(hDevice);
	DEBUG_RESULT("Failed to access ADIN1110", result, 0);

	LwIP_StructInit(&myConn, hDevice, boardDetails.mac);
	LwIP_Init(&myConn, &boardDetails);
	LwIP_ADIN1110LinkInput(&myConn.netif);
	HAL_Delay(3000);
	netif_set_link_up(&myConn.netif);

#endif  /* USE_LWIP */

	uint32_t heartbeatCheckTime = 0;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	/* Uncomment these variable declarations when using lwIP stack*/
#ifdef USE_LWIP
	while (1) {
#endif  /* USE_LWIP */
#ifndef USE_LWIP
	while (!FRAME_COUNT || (txIdx < FRAME_COUNT)) {
	#endif  /* USE_LWIP */
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
#ifdef USE_LWIP
		uint32_t now = BSP_SysNow();
		        if ((now - heartbeatCheckTime) >= 250) {
		            heartbeatCheckTime = now;
		            BSP_HeartBeat();
		            sys_check_timeouts();
		        }
		        LwIP_ADIN1110LinkInput(&myConn.netif);

		        // Process the UDP query/response state machine.
		        process_udp_query();
		    }
#endif  /* USE_LWIP */

	#ifndef USE_LWIP
		uint32_t now = BSP_SysNow();

		if ((now - heartbeatCheckTime) >= 250) {
			heartbeatCheckTime = now;
			BSP_HeartBeat();
			heartbeatTicks++;

			if (heartbeatTicks >= 8) {
				heartbeatTicks = 0;
				printStats(hDevice);
			}
		}

		if (txBufAvailable[txBufDescIdx]) {

			memset(txBuf[txBufDescIdx], 0, MAX_FRAME_SIZE);

			memcpy(&txBuf[txBufDescIdx][0], dest_mac, 6);
			memcpy(&txBuf[txBufDescIdx][6], mySourceMac, 6);

			txBuf[txBufDescIdx][12] = (myEtherType >> 8) & 0xFF;
			txBuf[txBufDescIdx][13] = (myEtherType) & 0xFF;

			uint32_t *pIndex = (uint32_t*) &txBuf[txBufDescIdx][14];
			*pIndex = frameIdx;

			size_t payloadOffset = 18;
			memcpy(&txBuf[txBufDescIdx][payloadOffset], payloadStr, payloadLen);
			size_t totalLen = payloadOffset + payloadLen;
			if (totalLen < 64)
				totalLen = 64;

			txBufDesc[txBufDescIdx].pBuf = &txBuf[txBufDescIdx][0];
			txBufDesc[txBufDescIdx].trxSize = totalLen;
			txBufDesc[txBufDescIdx].bufSize = MAX_FRAME_BUF_SIZE;
			txBufDesc[txBufDescIdx].egressCapt = ADI_MAC_EGRESS_CAPTURE_NONE;
			txBufDesc[txBufDescIdx].cbFunc = txCallback;

			txBufAvailable[txBufDescIdx] = false;

			adi_eth_Result_e res = adin1110_SubmitTxBuffer(hDevice,
					&txBufDesc[txBufDescIdx]);

			if (res == ADI_ETH_SUCCESS) {
				txBufDescIdx = (txBufDescIdx + 1) % BUFF_DESC_COUNT;
				frameIdx++;
			} else {
				txBufAvailable[txBufDescIdx] = true;
			}
		}
	}

	while (rxIdx < FRAME_COUNT)
		;
	while (rxIdx < txIdx)
		;

	printStats(hDevice);
	adi_eth_Result_e uninitRes = adin1110_UnInit(hDevice);
	DEBUG_RESULT("adin1110_UnInit", uninitRes, ADI_ETH_SUCCESS);
	while (1) {
	}
	#endif  /* USE_LWIP */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
 * @brief GPIO EXTI Callback
 *
 * This function is called by the HAL library when a GPIO EXTI interrupt occurs.
 * It handles the ADIN1110's INT_N interrupt by invoking the driver's interrupt handler.
 *
 * @param GPIO_Pin The GPIO pin number that triggered the interrupt.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == Reset_Button_Pin) {
		DEBUG_MESSAGE("Reset button interrupt triggered.\r\n");
		uint32_t pressStart = HAL_GetTick();
		while (HAL_GPIO_ReadPin(Reset_Button_GPIO_Port, Reset_Button_Pin)
				== GPIO_PIN_RESET) {
			if ((HAL_GetTick() - pressStart) >= 3000) {
				DEBUG_MESSAGE(
						"User button long press detected: Initiating reset.\r\n");
				BSP_HWReset(true);
				NVIC_SystemReset();
				break;
			}
		}
	}
	if (GPIO_Pin == ETH_INT_N_Pin) {

		HAL_INT_N_DisableIRQ();
		adi_eth_Result_e result = adin1110_HandleInterrupt(hDevice);
		if (result != ADI_ETH_SUCCESS) {
		}
		HAL_INT_N_EnableIRQ();
	}

	if (GPIO_Pin == Interrupt_Pin) {
		MX_Led_Toggle();
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */

  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
