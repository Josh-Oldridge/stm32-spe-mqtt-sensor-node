/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_def.h
  * @brief   BSP Definitions for CN0575 SPE Board Project
  * @details Contains additional GPIO pin definitions for the STM32L496ZG-P Nucleo board in
  *          the CN0575 Single Pair Ethernet (SPE) board project. Originally intended as main.h,
  *          it defines pins for EEPROM and configuration (CFG0-3), but only a subset (e.g.,
  *          CFG pins) may be used as placeholders or for future expansion. Most active pin
  *          definitions are in main.h and bsp_config.h.
  * @addtogroup bsp Board Support Package
  * @{
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/**
  *  Portions Copyright (c) 2020, 2021 Analog Devices, Inc.
  */

/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BSP_DEF_H
#define BSP_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/*
 * Note: EEPROM pins (CS, MISO, SCK) and AltSS are defined but unused in the CN0575 project;
 *       SPI1 for ADIN1110 uses different pins (PA5, PA6, PA7) from main.h and bsp_config.h.
 */
#define EEPROM_CS_Pin             GPIO_PIN_4
#define EEPROM_CS_GPIO_Port       GPIOA
#define EEPROM_MISO_Pin           GPIO_PIN_7
#define EEPROM_MISO_GPIO_Port     GPIOA
#define EEPROM_SCK_Pin            GPIO_PIN_5
#define EEPROM_SCK_GPIO_Port      GPIOA
#define EEPROM_MISOA6_Pin         GPIO_PIN_6
#define EEPROM_MISOA6_GPIO_Port   GPIOA
#define CFG2_Pin                  GPIO_PIN_2
#define CFG2_GPIO_Port            GPIOB
/*
 * Note: CFG0-3 pins (PB0-PB2, PB5) are defined for configuration but only CFG0 (PB0) is read
 *       in BSP_getConfigPins; their usage is minimal or placeholder in the CN0575 project.
 */
#define CFG3_Pin                  GPIO_PIN_5
#define CFG3_GPIO_Port            GPIOB
#define AltSS_Pin                 GPIO_PIN_9
#define CFG0_Pin                  GPIO_PIN_0
#define CFG0_GPIO_Port            GPIOB
#define CFG1_Pin                  GPIO_PIN_1
#define CFG1_GPIO_Port            GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* BSP_DEF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
