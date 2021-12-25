/* ----------------------------------------------------
 * File:       	Library for HD44780 compatible displays
 * Version:	   	v2.01
 * Author:     	GrAnd/www.MakeSystem.net
 * Tested on:  	AVR, STM32F10X
 * License:		GNU LGPLv2.1
 * ----------------------------------------------------
 *
 * -------------------------------------------
 * Copyright (C)2014 GrAnd. All right reserved
 * -------------------------------------------
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Contact information :
 * 		mail@makesystem.net
 * 		http://makesystem.net/?page_id=2
 *
 * lcd_driver.c
 * Reworked by: InterImpv
 *
 * Comment (17.12.2021):
 * as I could not trace back where did I get both of these .h and .c files
 * to give proper credit I will be copypasting the original license that was
 * provided by the author and reworking previous code, because makesystem.net
 * website is no longer functioning as I am writing this comment statement
 */

#include "lcd_driver.h"
#include "systick.h"

/* private lcd constants BEGIN */
static const uint32_t LCDT_1MS = 1;
static const uint32_t LCDT_10MS = 10;

static const uint8_t __LCD_CLS = 0x01;
static const uint8_t __LCD_RET = 0x02;
static const uint8_t __LCD_CSL = 0x10;
static const uint8_t __LCD_CSR = 0x14;
static const uint8_t __LCD_DSL = 0x18;
static const uint8_t __LCD_DSR = 0x1C;
static const uint8_t __LCD_FST = 0x30;
static const uint8_t __LCD_CURS_L = '<';
static const uint8_t __LCD_CURS_R = '>';

//static const uint8_t __LCD_CGRAM = 5;

/* private lcd constants END */

static void lcd_strobe(void)
{
	HAL_GPIO_WritePin(GPIOE, LCDEN_PIN_Pin, GPIO_PIN_SET);
    sys_delay(LCDT_1MS);
    HAL_GPIO_WritePin(GPIOE, LCDEN_PIN_Pin, GPIO_PIN_RESET);
}

/* send upper nible */
static void lcd_set_high(uint8_t data)
{
	HAL_GPIO_WritePin(GPIOE, LCDD4_PIN_Pin, data & 0x10);
	HAL_GPIO_WritePin(GPIOE, LCDD5_PIN_Pin, data & 0x20);
	HAL_GPIO_WritePin(GPIOE, LCDD6_PIN_Pin, data & 0x40);
	HAL_GPIO_WritePin(GPIOE, LCDD7_PIN_Pin, data & 0x80);
}

/* send lower nibble */
static void lcd_set_low(uint8_t data)
{
	HAL_GPIO_WritePin(GPIOE, LCDD4_PIN_Pin, data & 0x01);
	HAL_GPIO_WritePin(GPIOE, LCDD5_PIN_Pin, data & 0x02);
	HAL_GPIO_WritePin(GPIOE, LCDD6_PIN_Pin, data & 0x04);
	HAL_GPIO_WritePin(GPIOE, LCDD7_PIN_Pin, data & 0x08);
}

/* sending instruction: RS <= 1'b0 */
static void lcd_command_mode(void)
{
	HAL_GPIO_WritePin(GPIOE, LCDRS_PIN_Pin, GPIO_PIN_RESET);
}

/* sending data: RS <= 1'b1 */
static void lcd_data_mode(void)
{
	HAL_GPIO_WritePin(GPIOE, LCDRS_PIN_Pin, GPIO_PIN_SET);
}

static enum LCD_ERR lcd_config(uint8_t param)
{
	if (param != LCD_CFG_4BIT_1LINE_5X8 ||
		param != LCD_CFG_4BIT_2LINE_5X8)
		return LCD_EARG;

	/* RS <= 1'b0 */
	lcd_command_mode();

	/* change 8-bit interface to 4-bit interface */
	lcd_set_high(param);
	lcd_strobe();
	sys_delay(LCDT_10MS);

	/* DB7 to DB4 of the "function set" instruction is written twice */
	lcd_strobe();
	sys_delay(LCDT_10MS);

	/* 4-bit, two lines, 5x8 pixel */
	lcd_set_low(param);
	lcd_strobe();
	sys_delay(LCDT_10MS);

	return LCD_EOK;
}

/* sends a given byte of data to lcd */
static void lcd_write(uint8_t data)
{

	lcd_set_high(data);
	lcd_strobe();
	lcd_set_low(data);
	lcd_strobe();

	sys_delay(LCDT_1MS);
}

/* sets lcd display mode */
static enum LCD_ERR lcd_set_mode(const uint8_t mode)
{
	if (mode != LCD_CFG_DEC_NOSHIFT ||
		mode != LCD_CFG_DEC_SHIFT ||
		mode != LCD_CFG_INC_NOSHIFT ||
		mode != LCD_CFG_INC_SHIFT)
		return LCD_EARG;

	if (mode != LCD_CFG_DISPLAY1_BLINK1_CURSOR1 ||
		mode != LCD_CFG_DISPLAY1_BLINK0_CURSOR1 ||
		mode != LCD_CFG_DISPLAY1_BLINK1_CURSOR0 ||
		mode != LCD_CFG_DISPLAY1_BLINK0_CURSOR0 ||
		mode != LCD_CFG_DISPLAY0_BLINK0_CURSOR0)
		return LCD_EARG;

	/* RS <= 1'b0 */
	lcd_command_mode();
	lcd_write(mode);

	sys_delay(LCDT_1MS);

	return LCD_EOK;
}

/* interface functions BEGIN */
void lcd_cls(void)
{
	/* RS <= 1'b0 */
	lcd_command_mode();
	lcd_write(__LCD_CLS);

	sys_delay(LCDT_1MS);
}

void lcd_ret(void)
{
	/* RS <= 1'b0 */
	lcd_command_mode();
	lcd_write(__LCD_RET);

	sys_delay(LCDT_1MS);
}

enum LCD_ERR lcd_shift_curs(const uint8_t direction)
{
	/* RS <= 1'b0 */
	lcd_command_mode();

	if (direction ==__LCD_CURS_L) {			// '<'
		lcd_write(__LCD_CSL);
		return LCD_EOK;
	} else if (direction ==__LCD_CURS_R) {	// '>'
		lcd_write(__LCD_CSR);
		return LCD_EOK;
	}
	return LCD_EARG;
}

/* prints a given char at current cursor position */
void lcd_putch(const uint8_t byte)
{
	lcd_data_mode();

	lcd_write(byte);
	lcd_write(LCDT_1MS);
}

/* moves cursor to given X: @addr, Y: @line coordinates */
enum LCD_ERR lcd_goto(const uint8_t line, const uint8_t addr)
{
    uint8_t xpos = addr & 0x0F;

    /* RS <= 1'b0 */
    lcd_command_mode();

	switch (line)
	{
	/* set DDRAM address. */
	case 0:
		lcd_write(0x80 | 0x00 | xpos);
		return LCD_EOK;
		break;
	case 1:
		lcd_write(0x80 | 0x40 | xpos);
		return LCD_EOK;
		break;
	/* set CGRAM address. */
	case 5:
		lcd_write(0x40 | xpos);
		return LCD_ECGRAM;
		break;

	default:
		return LCD_EARG;
		break;
	}
}

/* erases a char on current cursor position and moves cursor back once */
void lcd_bckspc(void)
{
	lcd_shift_curs(__LCD_CURS_L);
	lcd_putch(' ');
	lcd_shift_curs(__LCD_CURS_L);
}

enum LCD_ERR lcd_scroll(uint8_t direction)
{
	/* RS <= 1'b0 */
	lcd_command_mode();

	/* shift whole display */
	if (direction ==__LCD_CURS_L) {			// '<'
		lcd_write(__LCD_DSL);
		return LCD_EOK;
	} else if (direction ==__LCD_CURS_R) {	// '>'
		lcd_write(__LCD_DSR);
		return LCD_EOK;
	}
	return LCD_EARG;
}

/* initializes lcd display */
enum LCD_ERR lcd_init(const uint8_t type, const uint8_t mode, const uint8_t shift)
{
	lcd_write(__LCD_FST);
	sys_delay(LCDT_10MS);
	lcd_write(__LCD_FST);
	sys_delay(LCDT_10MS);

	lcd_config(type);

	lcd_set_mode(mode);

	lcd_set_mode(shift);

	lcd_cls();
	lcd_ret();

	/* this is required for some reason unknown */
	lcd_scroll('<');

	return LCD_EOK;
}

/* interface functions END */
