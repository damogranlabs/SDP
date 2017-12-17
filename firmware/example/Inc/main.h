/**
  ******************************************************************************
  * File Name          : main.hpp
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_ll_crc.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_dma.h"

#include "stm32f0xx_ll_exti.h"

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define GPIO_1_Pin GPIO_PIN_2
#define GPIO_1_GPIO_Port GPIOE
#define GPIO_2_Pin GPIO_PIN_3
#define GPIO_2_GPIO_Port GPIOE
#define NRF24L01_IRQ_Pin GPIO_PIN_4
#define NRF24L01_IRQ_GPIO_Port GPIOE
#define GPIO_7_Pin GPIO_PIN_14
#define GPIO_7_GPIO_Port GPIOC
#define GPIO_8_Pin GPIO_PIN_15
#define GPIO_8_GPIO_Port GPIOC
#define AD_IN_1_Pin GPIO_PIN_0
#define AD_IN_1_GPIO_Port GPIOC
#define AD_IN_2_Pin GPIO_PIN_1
#define AD_IN_2_GPIO_Port GPIOC
#define AD_IN_3_Pin GPIO_PIN_2
#define AD_IN_3_GPIO_Port GPIOC
#define AD_IN_4_Pin GPIO_PIN_3
#define AD_IN_4_GPIO_Port GPIOC
#define IN_1_Pin GPIO_PIN_1
#define IN_1_GPIO_Port GPIOA
#define IN_2_Pin GPIO_PIN_2
#define IN_2_GPIO_Port GPIOA
#define IN_3_Pin GPIO_PIN_3
#define IN_3_GPIO_Port GPIOA
#define IN_4_Pin GPIO_PIN_4
#define IN_4_GPIO_Port GPIOA
#define OUT_1_Pin GPIO_PIN_5
#define OUT_1_GPIO_Port GPIOA
#define OUT_2_Pin GPIO_PIN_6
#define OUT_2_GPIO_Port GPIOA
#define OUT_3_Pin GPIO_PIN_7
#define OUT_3_GPIO_Port GPIOA
#define OUT_4_Pin GPIO_PIN_4
#define OUT_4_GPIO_Port GPIOC
#define FL_1_Pin GPIO_PIN_5
#define FL_1_GPIO_Port GPIOC
#define FL_2_Pin GPIO_PIN_0
#define FL_2_GPIO_Port GPIOB
#define FL_3_Pin GPIO_PIN_1
#define FL_3_GPIO_Port GPIOB
#define FL_4_Pin GPIO_PIN_7
#define FL_4_GPIO_Port GPIOE
#define FL_5_Pin GPIO_PIN_8
#define FL_5_GPIO_Port GPIOE
#define FL_6_Pin GPIO_PIN_9
#define FL_6_GPIO_Port GPIOE
#define FL_7_Pin GPIO_PIN_10
#define FL_7_GPIO_Port GPIOE
#define FL_8_Pin GPIO_PIN_11
#define FL_8_GPIO_Port GPIOE
#define NRF24L01_CSN_Pin GPIO_PIN_12
#define NRF24L01_CSN_GPIO_Port GPIOE
#define NRF24L01_SCK_Pin GPIO_PIN_13
#define NRF24L01_SCK_GPIO_Port GPIOE
#define NRF24L01_MISO_Pin GPIO_PIN_14
#define NRF24L01_MISO_GPIO_Port GPIOE
#define NRF24L01_MOSI_Pin GPIO_PIN_15
#define NRF24L01_MOSI_GPIO_Port GPIOE
#define NRF24L01_CE_Pin GPIO_PIN_11
#define NRF24L01_CE_GPIO_Port GPIOB
#define I2C_IN_SCL_Pin GPIO_PIN_13
#define I2C_IN_SCL_GPIO_Port GPIOB
#define I2C_IN_SDA_Pin GPIO_PIN_14
#define I2C_IN_SDA_GPIO_Port GPIOB
#define I2C_IN_INT_Pin GPIO_PIN_15
#define I2C_IN_INT_GPIO_Port GPIOB
#define I2C_IN_INT_EXTI_IRQn EXTI4_15_IRQn
#define RS422_TX_Pin GPIO_PIN_8
#define RS422_TX_GPIO_Port GPIOD
#define RS422_RX_Pin GPIO_PIN_9
#define RS422_RX_GPIO_Port GPIOD
#define SYSTEM_STATUS_Pin GPIO_PIN_10
#define SYSTEM_STATUS_GPIO_Port GPIOD
#define RS422_CTS_Pin GPIO_PIN_11
#define RS422_CTS_GPIO_Port GPIOD
#define RS422_RTS_Pin GPIO_PIN_12
#define RS422_RTS_GPIO_Port GPIOD
#define DBG_OUT_1_Pin GPIO_PIN_13
#define DBG_OUT_1_GPIO_Port GPIOD
#define DBG_OUT_2_Pin GPIO_PIN_14
#define DBG_OUT_2_GPIO_Port GPIOD
#define DBG_IN_1_Pin GPIO_PIN_15
#define DBG_IN_1_GPIO_Port GPIOD
#define DBG_IN_2_Pin GPIO_PIN_6
#define DBG_IN_2_GPIO_Port GPIOC
#define BUZZER_Pin GPIO_PIN_7
#define BUZZER_GPIO_Port GPIOC
#define UART_STATUS_Pin GPIO_PIN_9
#define UART_STATUS_GPIO_Port GPIOC
#define UART_STATUS_EXTI_IRQn EXTI4_15_IRQn
#define UART_RESET_Pin GPIO_PIN_8
#define UART_RESET_GPIO_Port GPIOA
#define UART_TX_Pin GPIO_PIN_9
#define UART_TX_GPIO_Port GPIOA
#define UART_RX_Pin GPIO_PIN_10
#define UART_RX_GPIO_Port GPIOA
#define UART_EXT_TX_Pin GPIO_PIN_10
#define UART_EXT_TX_GPIO_Port GPIOC
#define UART_EXT_RX_Pin GPIO_PIN_11
#define UART_EXT_RX_GPIO_Port GPIOC
#define LCD_D7_Pin GPIO_PIN_0
#define LCD_D7_GPIO_Port GPIOD
#define LCD_D6_Pin GPIO_PIN_1
#define LCD_D6_GPIO_Port GPIOD
#define LCD_D5_Pin GPIO_PIN_2
#define LCD_D5_GPIO_Port GPIOD
#define LCD_D4_Pin GPIO_PIN_3
#define LCD_D4_GPIO_Port GPIOD
#define LCD_BACKLIGHT_Pin GPIO_PIN_4
#define LCD_BACKLIGHT_GPIO_Port GPIOD
#define LCD_E_Pin GPIO_PIN_5
#define LCD_E_GPIO_Port GPIOD
#define LCD_RS_Pin GPIO_PIN_6
#define LCD_RS_GPIO_Port GPIOD
#define I2C_OUT_INT_Pin GPIO_PIN_5
#define I2C_OUT_INT_GPIO_Port GPIOB
#define I2C_OUT_SCL_Pin GPIO_PIN_6
#define I2C_OUT_SCL_GPIO_Port GPIOB
#define I2C_OUT_SDA_Pin GPIO_PIN_7
#define I2C_OUT_SDA_GPIO_Port GPIOB
#define A_1_Pin GPIO_PIN_8
#define A_1_GPIO_Port GPIOB
#define A_2_Pin GPIO_PIN_9
#define A_2_GPIO_Port GPIOB
#define A_3_Pin GPIO_PIN_0
#define A_3_GPIO_Port GPIOE
#define A_4_Pin GPIO_PIN_1
#define A_4_GPIO_Port GPIOE

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
 #define USE_FULL_ASSERT    1U 

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
