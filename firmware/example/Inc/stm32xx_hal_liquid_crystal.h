/**
 ******************************************************************************
 * File Name          : stm32xx_hal_lliquid_crystal
 * Description        : This file provides code for the configuration
 *                      of HD44780 based LCD
 * @date    13-Aug-2017
 * @author  Domen Jurkovic, Damogran Labs
 * @source  http://damogranlabs.com/
 *          https://github.com/damogranlabs
 * @version v1.2
*/
 
#ifndef __LCD_H
#define __LCD_H

#include "main.h"
#include "stm32f0xx.h"

/********************************* USER SETUP DEFINES **************************************/
/* Pins should always be named:
 LCD_RS - register select pin
 LCD_E - enable pin 
 LCD_D4 - data pin 4
 LCD_D5 - data pin 5
 LCD_D6 - data pin 6
 LCD_D7 - data pin 7
 LCD_BACKLIGHT - backlight control pin
*/

//#define LCD_GO_TO_NEW_LINE_IF_STRING_TOO_LONG	// uncomment if strings larger than screen size should break and continue on new line.
#define LCD_WINDOW_PRINT_DELAY	800	// delay between static view and window scrolling (used in LCD_PrintStringWindow();)
//#define LCD_CONTROL_BACKLIGHT // un/comment if backlight is controlled with uC
  #define LCD_BACKLIGHT_TIMEOUT 5000  //[ms]
																
void LCD_Init(uint8_t rows, uint8_t cols);
void LCD_PrintString(uint8_t y, uint8_t x, char* str);
void LCD_PrintStringWindow(uint8_t y, uint8_t x, uint8_t window_size, uint16_t speed_ms, char* str);
void LCD_PrintNumber(uint8_t y, uint8_t x, int32_t number);
void LCD_PrintFloat(uint8_t y, uint8_t x, float number_f, uint8_t precision);

void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(void);
void LCD_ClearArea(uint8_t y, uint8_t x_start, uint8_t x_end);
void LCD_BlinkOn(void);
void LCD_BlinkOff(void);
void LCD_CursorOn(void);
void LCD_CursorOff(void);
void LCD_ScrollLeft(void);
void LCD_ScrollRight(void);

void LCD_BacklightOn(void);
void LCD_BacklightOff(void);

/**********************************************************/
/*	CUSTOM CHARACTER FUNCTIONS */
/**********************************************************/
/*
	Creates custom character at specific location 
	LCD supports up to 8 custom characters, locations: 0 - 7
	*data: Pointer to 8-bytes of data for one character
 */
void LCD_CreateChar(uint8_t location, uint8_t* data);

/**
 *  Puts custom created character on LCD
 *  location: Location on LCD where character is stored, 0 - 7
 */
void LCD_PutCustom(uint8_t y, uint8_t x, uint8_t location);


/**********************************************************/
/*	PRIVATE FUNCTIONS */
/**********************************************************/
void _lcd_init_pins(void);
void _lcd_send_command(uint8_t cmd);
void _lcd_send_command_4_bit(uint8_t cmd);
void _lcd_send_data(uint8_t data);
void _lcd_cursor_set(uint8_t row, uint8_t col);
void _lcd_enable_pulse(void);


#endif /* __LCD_H */
