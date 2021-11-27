/*
 * lcd_driver.h
 *
 *  Created on: Nov 24, 2021
 *      Author: InterImpv
 */

#ifndef INC_LCD_DRIVER_H_
#define INC_LCD_DRIVER_H_

#include "stm32f4xx_hal.h"

/* public lcd constants BEGIN */

#define LCDRS_PIN_Pin GPIO_PIN_7
#define LCDRW_PIN_Pin GPIO_PIN_10
#define LCDEN_PIN_Pin GPIO_PIN_11

#define LCDD4_PIN_Pin GPIO_PIN_12
#define LCDD5_PIN_Pin GPIO_PIN_13
#define LCDD6_PIN_Pin GPIO_PIN_14
#define LCDD7_PIN_Pin GPIO_PIN_15

#define LCD_CFG_4BIT_1LINE_5X8 	0x20u
#define LCD_CFG_4BIT_2LINE_5X8	0x28u

#define LCD_CFG_DEC_NOSHIFT 0x04u
#define LCD_CFG_DEC_SHIFT 	0x05u
#define LCD_CFG_INC_NOSHIFT 0x06u
#define LCD_CFG_INC_SHIFT 	0x07u

#define LCD_CFG_DISPLAY1_BLINK1_CURSOR1 0x0Fu
#define LCD_CFG_DISPLAY1_BLINK0_CURSOR1 0x0Eu
#define LCD_CFG_DISPLAY1_BLINK1_CURSOR0 0x0Du
#define LCD_CFG_DISPLAY1_BLINK0_CURSOR0 0x0Cu
#define LCD_CFG_DISPLAY0_BLINK0_CURSOR0 0x08u

/* public lcd constants END */

enum LCD_ERR {
	LCD_EOK = 0,
	LCD_EARG = 1,
	LCD_ECGRAM = 2
};

/* interface functions BEGIN */

/* clears the display */
void lcd_cls(void);

/* returns cursor to (0, 0) */
void lcd_ret(void);

/* shifts cursor one position left(<) or right(>) */
enum LCD_ERR lcd_shift_curs(const uint8_t direction);

/* shifts whole display one position left(<) or right(>) */
enum LCD_ERR lcd_scroll(uint8_t direction);

/* prints a given char at current cursor position */
void lcd_putch(const uint8_t byte);

/* moves cursor to given X: @addr, Y: @line coordinates */
enum LCD_ERR lcd_goto(const uint8_t line, const uint8_t addr);

/* erases a char on current cursor position and moves cursor back once */
void lcd_bckspc(void);

/* initializes lcd display */
enum LCD_ERR lcd_init(const uint8_t type, const uint8_t mode, const uint8_t shift);


/* interface functions END */

#endif /* INC_LCD_DRIVER_H_ */
