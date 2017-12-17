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
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DELAY_US_H
#define __DELAY_US_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
   
void delay_us_init(void);
	 
void delay_us(uint32_t micros);

#ifdef __cplusplus
}
#endif

#endif /* __DELAY_US_H */

