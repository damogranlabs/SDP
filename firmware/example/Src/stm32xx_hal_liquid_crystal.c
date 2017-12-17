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
 
	1. Set up library:
		1.1. Set up pins using HAL library or CubeMX. See pin naming in header file.
		1.2. Set up (comment/uncomment) defines in header file
	
	2. Init library:
		LCD_Init(2, 20);	// 2 rows, 20 characters
	
	3. Print characters/strings/numbers:
  Note: x and y location starts with 0!
		LCD_PrintString(0, 0, "www.damogranlabs.com ");
		LCD_PrintStringWindow(0, 0, 10, 350, "Find us on github and www.damogranlabs.com ");
		LCD_PrintNumber(1, 0, -10);
		LCD_PrintFloat(1, 0, -326.5635, 5);
		
	3.1 Check header file for more functions. 
	
	3.2. Create & print custom characters
		uint8_t damogranlabs_logo[]={
			0x0F,
			0x13,
			0x11,
			0x11,
			0x0e,
			0x00,
			0x00		
		};
		LCD_CreateChar(0, damogranlabs_logo); 
		LCD_PutCustom(0, 9, 0);
 */
 
/* Includes -------------------------------------*/
#include "stm32xx_hal_liquid_crystal.h"
#include "stm32xx_hal_delay_us.h"

#include <stdio.h>
#include <string.h>

/* Private variables -------------------------------------*/
typedef struct {
	uint8_t DisplayControl;
	uint8_t DisplayFunction;
	uint8_t DisplayMode;
	uint8_t Rows;
	uint8_t Cols;
	uint8_t currentX;
	uint8_t currentY;
} _lcd_options_t;		// private LCD structure

static _lcd_options_t _lcd_options;
static uint32_t backlight_start_time = 0; // LCD backlight start time


/* Private defines -------------------------------------*/
/* Commands*/
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

/* Flags for display entry mode */
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/* Flags for display on/off control */
#define LCD_DISPLAYON           0x04
#define LCD_CURSORON            0x02
#define LCD_BLINKON             0x01

/* Flags for display/cursor shift */
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

/* Flags for function set */
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00

/**
  * @brief  Initializes LCD (HD44780)
  * @param  rows - height of lcd (>= 1)
  * @param  cols - width of lcdNone (>= 1)
  */
void LCD_Init(uint8_t rows, uint8_t cols) {
	delay_us_init();				// Initialize microsecond delay 
	
	// Set LCD width and height 
	_lcd_options.Rows = rows;
	_lcd_options.Cols = cols;
	// Set cursor pointer to beginning for LCD 
	_lcd_options.currentX = 0;
	_lcd_options.currentY = 0;
	
	_lcd_options.DisplayFunction = LCD_4BITMODE | LCD_5x8DOTS | LCD_1LINE;
	if (rows > 1) {
		_lcd_options.DisplayFunction |= LCD_2LINE;
	}
	
	/* Try to set 4bit mode */
	_lcd_send_command_4_bit(0x03);
	HAL_Delay(5);
	
	/* Second try */
	_lcd_send_command_4_bit(0x03);
	HAL_Delay(5);
	
	/* Third goo! */
	_lcd_send_command_4_bit(0x03);
	HAL_Delay(5);	
	
	/* Set 4-bit interface */
	_lcd_send_command_4_bit(0x02);
	delay_us(100);
	
	// Set # lines, font size, etc.
	_lcd_send_command(LCD_FUNCTIONSET | _lcd_options.DisplayFunction);

	// Turn the display on, no cursor, no blinking
	_lcd_options.DisplayControl = LCD_DISPLAYON;
	LCD_DisplayOn();

	LCD_Clear();

	// Default font & direction
	_lcd_options.DisplayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	_lcd_send_command(LCD_ENTRYMODESET | _lcd_options.DisplayMode);
	HAL_Delay(5);
}

/**
  * @brief  Print string on lcd
  * @param  y - row (starts with 0)
  * @param  x - column  (starts with 0)
  * @param  *str - pointer to string to display
  */
void LCD_PrintString(uint8_t y, uint8_t x, char* str) {
  #ifdef LCD_CONTROL_BACKLIGHT
    backlight_start_time = HAL_GetTick();
    HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET);
  #endif
  
	_lcd_cursor_set(y, x);
	while (*str) {
		#ifdef LCD_GO_TO_NEW_LINE_IF_STRING_TOO_LONG
			if (_lcd_options.currentX >= _lcd_options.Cols) {
				_lcd_options.currentX = 0;
				_lcd_options.currentY++;
				_lcd_cursor_set(_lcd_options.currentY, _lcd_options.currentX);
			}
			if (*str == '\n') {
				_lcd_options.currentY++;
				_lcd_cursor_set(_lcd_options.currentY, _lcd_options.currentX);
			} else if (*str == '\r') {
				_lcd_cursor_set(_lcd_options.currentY, 0);
			} else {
				_lcd_send_data(*str);
				_lcd_options.currentX++;
			}
			str++;
		#else
			if (*str == '\n') {
				_lcd_options.currentY++;
				_lcd_cursor_set(_lcd_options.currentY, _lcd_options.currentX);
			} else if (*str == '\r') {
				_lcd_cursor_set(_lcd_options.currentY, 0);
			} else {
				_lcd_send_data(*str);
				_lcd_options.currentX++;
			}
			str++;
		#endif
		
	}
}

/**
  * @brief  Print string and scroll it (right to left) on LCD in specific window size.
  * @param  window_size - number of characters from x position, where string will be displayed
  * @param  y - row (starts with 0)
  * @param  x - column  (starts with 0)
  * @param  *str - pointer to string to display
  */
void LCD_PrintStringWindow(uint8_t y, uint8_t x, uint8_t window_size, uint16_t speed_ms, char* str){

	uint8_t _window_character_number = 0;
	uint8_t string_length = strlen(str); // number of characters in passed string
	uint8_t _str_character_number = 0;	// 0 - strlen(str)
	char* _str = str;
	
	_lcd_cursor_set(y, x);
	
	if(string_length > window_size){	// string is larger than window size. String must be scrolled 
		#ifdef LCD_CONTROL_BACKLIGHT
      HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET);
      backlight_start_time = HAL_GetTick();
    #endif
    
		// write character while they are inside window size
		while(_str_character_number < window_size){	
			_lcd_send_data(*_str);
			
			_lcd_options.currentX++;
			_str_character_number++;
			_str++;
		}
		HAL_Delay(LCD_WINDOW_PRINT_DELAY);
			
		_str_character_number = 0;
		_str = str++;	// increment starting character
		
		// scroll characters in window until last x characters can be shown in window
		while((string_length - _str_character_number) >= window_size){
			_window_character_number = 0;	// reset character position in window.
			_lcd_cursor_set(y, x);
						
			while(_window_character_number < window_size){				// while character number is smaller than window size
				_lcd_send_data(*_str);	// print character
				_lcd_options.currentX++;	// increment x position
				_window_character_number++;	// increment position in window
				_str++;	// increment starting character
			}
			
			_str = str++;	// increment starting character
			_str_character_number++;
			
			HAL_Delay(speed_ms);
		}
	}
	else{	// string is smaller than window size. Print it normally.
		LCD_PrintString(y, x, str);
	}
}

/**
  * @brief  Print string and scroll it (right to left) on LCD in specific window size.
  * @param  y - row (starts with 0)
  * @param  x - column  (starts with 0)
  * @param  number - range: -2147483647 to 2147483647
  */
void LCD_PrintNumber(uint8_t y, uint8_t x, int32_t number){
	char buf[50];
  snprintf (buf, 100, "%d", number);
	LCD_PrintString(y, x, buf);
}

/**
  * @brief  Print string and scroll it (right to left) on LCD in specific window size.
  * @param  y - row
  * @param  x - column
  * @param  number_f - float number
  * @param  precision - number of digits to be displayed
  */
void LCD_PrintFloat(uint8_t y, uint8_t x, float number_f, uint8_t precision){
	char buf[50];
  snprintf ( buf, 100, "%.*g", precision, number_f);
	LCD_PrintString(y, x, buf);
}

void LCD_Clear(void) {
	_lcd_send_command(LCD_CLEARDISPLAY);
	HAL_Delay(3);
  
  #ifdef LCD_CONTROL_BACKLIGHT
    HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);
  #endif
}

void LCD_ClearArea(uint8_t y, uint8_t x_start, uint8_t x_end){
	uint8_t x = x_start;
	while(x <= x_end){
		LCD_PrintString(y, x, " ");
		x++;
	}
	
}

void LCD_DisplayOn(void) {
	_lcd_options.DisplayControl |= LCD_DISPLAYON;
	_lcd_send_command(LCD_DISPLAYCONTROL | _lcd_options.DisplayControl);
}

void LCD_DisplayOff(void) {
	_lcd_options.DisplayControl &= ~LCD_DISPLAYON;
	_lcd_send_command(LCD_DISPLAYCONTROL | _lcd_options.DisplayControl);
  
  LCD_BacklightOff();
  #ifdef LCD_CONTROL_BACKLIGHT
    HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);
  #endif
}

void LCD_BlinkOn(void) {
	_lcd_options.DisplayControl |= LCD_BLINKON;
	_lcd_send_command(LCD_DISPLAYCONTROL | _lcd_options.DisplayControl);
}

void LCD_BlinkOff(void) {
	_lcd_options.DisplayControl &= ~LCD_BLINKON;
	_lcd_send_command(LCD_DISPLAYCONTROL | _lcd_options.DisplayControl);
}

void LCD_CursorOn(void) {
	_lcd_options.DisplayControl |= LCD_CURSORON;
	_lcd_send_command(LCD_DISPLAYCONTROL | _lcd_options.DisplayControl);
}

void LCD_CursorOff(void) {
	_lcd_options.DisplayControl &= ~LCD_CURSORON;
	_lcd_send_command(LCD_DISPLAYCONTROL | _lcd_options.DisplayControl);
}

void LCD_ScrollLeft(void) {
	_lcd_send_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LCD_ScrollRight(void) {
	_lcd_send_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void LCD_BacklightOn(void){
  HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);
}

void LCD_BacklightOff(void){
  #ifdef LCD_CONTROL_BACKLIGHT
    if(HAL_GetTick() > (backlight_start_time + LCD_BACKLIGHT_TIMEOUT)){
      HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);
    }
  #endif
}

void LCD_CreateChar(uint8_t location, uint8_t *data) {
	uint8_t i;
	/* We have 8 locations available for custom characters */
	location &= 0x07;
	_lcd_send_command(LCD_SETCGRAMADDR | (location << 3));
	
	for (i = 0; i < 8; i++) {
		_lcd_send_data(data[i]);
	}
}

void LCD_PutCustom(uint8_t y, uint8_t x, uint8_t location) {
	_lcd_cursor_set(y, x);
	_lcd_send_data(location);
}

/* Private functions */
void _lcd_send_command(uint8_t cmd) {
	/* Command mode */
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
		
	/* High nibble */
	_lcd_send_command_4_bit(cmd >> 4);
	/* Low nibble */
	_lcd_send_command_4_bit(cmd & 0x0F);
}

void _lcd_send_data(uint8_t data) {
	/* Data mode */
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
	
	/* High nibble */
	_lcd_send_command_4_bit(data >> 4);
	/* Low nibble */
	_lcd_send_command_4_bit(data & 0x0F);
}

void _lcd_send_command_4_bit(uint8_t cmd) {
	/* Set output port */
	HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (GPIO_PinState) (cmd & 0x08));
	HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (GPIO_PinState) (cmd & 0x04));
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (GPIO_PinState) (cmd & 0x02));
	HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (GPIO_PinState) (cmd & 0x01));
	
	_lcd_enable_pulse();
}

void _lcd_cursor_set(uint8_t row, uint8_t col){
	uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
		
	/* Go to beginning */
	if (row >= _lcd_options.Rows) {
		row = 0;
	}
	
	/* Set current column and row */
	_lcd_options.currentX = col;
	_lcd_options.currentY = row;
	
	/* Set location address */
	_lcd_send_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void _lcd_init_pins(void) {
	// GPIO initial state
	HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, GPIO_PIN_RESET);
  
  #ifdef LCD_CONTROL_BACKLIGHT
    HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);
  #endif
}

void _lcd_enable_pulse(void){
	HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
	delay_us(2);
	HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
	delay_us(100);
}
