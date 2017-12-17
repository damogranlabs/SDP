/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Example project for Simple Data Protocol
  *                      
  * @date    3-Dec-2017
  * @author  Domen Jurkovic
  * @source  http://damogranlabs.com/
  *          https://github.com/damogranlabs
  *
  ******************************************************************************
  **
  * One node is initialised and connected to PC (PC is master) over UART. 
  * Library uses STM32 Low Layer UART and CRC drivers. (UART 1)
  * 
  * SDP is build upon this files:
  *   - sdp.h
  *   - sdp.c
  *   - sdp_user.c
  *     - ring_buffer.h
  *     - ring_buffer.c
  * 
  * Example program receives data from PC and turn on LED if transmission is successfull.
  * Test with PC node python bindings.
  *
  * Note: As this example is a cleaned code of some other, bigger project, 
  *       there could be some leftovers of its code (like additional UART3 driver or gpio).
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"
#include "crc.h"
#include "usart.h"
#include "gpio.h"

#include <string.h>
#include "sdp.h"

#include "stm32xx_hal_liquid_crystal.h"

#define EXAMPLE_SDP_CRC_POLYNOME 0x8005
#define UC_NODE_ID 0 // uc_node -> microcontroller node
#define UC_SDP_MAX_PAYLOAD  50  // each payload can contain maximum of 50 bytes 
#define UC_SDP_RX_BUFFER_COUNT  2  // rx buffer can store up to this number of messages
  // Note: this protocol requires a respond to every message before a new transmission can take place. 
  //       UC_SDP_RX_BUFFER_COUNT value serves to avoid any incomming garbage burst data. 
  //       Should be kept low to avoid large memory usage

SDP_uart_t uc_uart;
SDP_data_t uc_node;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

int main(void)
{
  /* MCU Configuration----------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();  
  
  // LL uart and CRC driver initialisation
  MX_USART1_UART_Init();
  MX_CRC_Init();
  
  // LCD INIT
  LCD_Init(2, 16);	// 2 rows, 16 characters
  HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET);  // enable LCD backlight

  // SPD node init
    // uart timeouts
  uc_uart.handle = USART1; // PC <-> CU use UART1
  uc_uart.rx_timeout = 2;   //[ms] (for receiving one byte) - not used in this example (stm32 LL  uart drivers does not support receive timeout)
  uc_uart.tx_timeout = 2;   //[ms] (for transmiting byte) - possible delay because of other interrupts with higher priority
    // CRC init
  LL_CRC_SetInitialData(CRC, 0);
  LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_16B);
  LL_CRC_SetPolynomialCoef(CRC, EXAMPLE_SDP_CRC_POLYNOME);
    // node initialisation (buffer timeouts
  if(sdp_init_node(&uc_node, &uc_uart, UC_NODE_ID, UC_SDP_MAX_PAYLOAD, UC_SDP_RX_BUFFER_COUNT) == false){
    LCD_PrintString(0, 0, "CU comm err");
    while(1);
  }
    // node message receive, response and transmitt timeouts
  uc_node.rx_msg_timeout = 100;  //[ms] - message must be received in this time
    // this value should include time delays of this node (other side, transmitting) higher priority nested interrupts
    // In our case, python should send out complete message in rx_msg_timeout
  uc_node.tx_msg_timeout = 20;  //[ms]
    // this value should include time delays of this node higher priority nested interrupts while transmitting data out
    // in our case, only uart RX_NE interrupt and systick has higher interrupt while transmitting data back to PC
  uc_node.response_timeout = 400;//[ms] - if this node is sending data to the other side, other side must respond in response_time
    // in our case, PC is master so this value is not used. However, if this node sends data to PC, PC must respond in response_time
  
  // enable RX_NE interrupt and frame, noise and other error interrupts
  LL_USART_EnableIT_RXNE(USART1);
  LL_USART_EnableIT_ERROR(USART1);
    
  // SYSTEM INIT OK
  LCD_PrintString(0, 0, "System init OK");
  
  
  while(1){

    sdp_parse_rx_data(&uc_node);      
      
  }
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

