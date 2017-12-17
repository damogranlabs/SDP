 /**
 ******************************************************************************
 * File Name          : stm32xx_hal_delay
 * Description        : This file provides code for the configuration
 *                      of microsecond delay.
 * @date    13-Aug-2017
 * @author  Domen Jurkovic, Damogran Labs
 * @source  http://damogranlabs.com/
 *          https://github.com/damogranlabs
 * @version v1.2
 *
 * @attention
 * This library doesn't generate precise us delay, so be careful - take a few us more/less.
 *
 *  1. initialize library with: delay_us_init();
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32xx_hal_delay_us.h"

/* Private variables ------------------------------------------------------------------*/
uint32_t us_multiplier;

/**
 * @brief Init microsecond delay
 * @note	for core clock frequency above 1MHz
 * @note	Doesn't generate precise us delay. For simple purposes.
 */
void delay_us_init(void) {
    us_multiplier = HAL_RCC_GetSysClockFreq() / 1000000; //For 1 us delay, we need to divide with 1M */
}

/**
 * @brief Delay for number of microsecond (aproximately)
 * @param	micros - number of microseconds (>= 1)
 */
void delay_us(uint32_t micros){
	micros *= us_multiplier;
	
  #pragma push
    #pragma diag_suppress 1267
    /* Here, the suppression fails because the push/pop must enclose the whole function. */ 
    //substitute 8 cycles for each call of asm code below == //micros /= 8; 
    __asm(" mov r0, micros \n"
    "loop: subs r0, #8 \n"
    " bhi loop \n");
  #pragma pop
  
}
