/*
 * terminal.c
 *
 *  Created on: Nov 24, 2021
 *      Author: InterImpv
 */

#include "terminal.h"
#include "lcd_driver.h"

#include <string.h>
#include <stdbool.h>

/* not used anywhere, but we can use this in conjuntion with @term_draw to force a redraw */
inline void term_request_update(term_win *term)
{
	term->flags.update_req = true;
}

inline void term_reject_update(term_win *term)
{
	term->flags.update_req = false;
}

/* sets cursor xpos to @x, ypos to @y */
void term_goto(term_win *term, const uint8_t x, const uint8_t y)
{
	term->cursor_x = x;
	term->cursor_y = y;
}

/* clears terminal primary buffer */
void term_cls(term_win *term)
{
	term_goto(term, 0, 0);    //return home

	memset(term->win, ' ', term->size);

	term_request_update(term);
}

/*
sends whole primary buffer to lcd display
this function is somewhat fast and sometimes lcd display can't keep up with it
so it actually redraws the screen only if @updateRequest flag of @std_win is set
if you need to actively redraw lcd screen every tick use @term_req_upd before calling this
 */
void term_draw(term_win *term)
{
	if (term->flags.update_req) {

		term_reject_update(term);

		for (uint8_t i = 0; i < term->rows; i++) {
			for (uint8_t j = 0; j < term->cols; j++) {
				lcd_goto(i, j);
				lcd_putch(term->win[i * term->cols + j]);
			}
		}
	}
}

/* prints given @ch on coordinates @x, @y */
void term_putchxy(term_win *term, const char ch, const uint8_t x, const uint8_t y)
{
	if ( term->cols > x && term->rows > y ) {
		uint8_t cursor_xprev = term->cursor_x;
		uint8_t cursor_yprev = term->cursor_y;

		term_goto(term, x, y);    // move cursors
		term->win[y * term->cols + x] = ch;
		term_goto(term, cursor_xprev, cursor_yprev); // reset cursor

		term_request_update(term);
	}
}

/* prints a @ch at current cursor position and advances cursor once */
void term_putch(term_win *term, const char ch)
{
	uint8_t x = term->cursor_x;
	uint8_t y = term->cursor_y;

	term->flags.h_overflow = false;
	term->flags.v_overflow = false;

	if (ch != '\n')
		term->win[y * term->cols + x] = ch;

	term_request_update(term);

	term->cursor_x++;

	/* line overflow */
	if (term->cursor_x > term->cols - 1 || ch == '\n') {
		term->cursor_x = 0;
		term->cursor_y++;

		term->flags.h_overflow = true;
	}

	/* vertical overflow */
	if (term->cursor_y > term->rows)
	{
		term->cursor_y = 0;

		term->flags.v_overflow = true;
	}
}

/* erases char at current cursor position and backs cursor once*/
void term_erasech(term_win *term)
{
	uint8_t x = term->cursor_x;
	uint8_t y = term->cursor_y;

	term->win[y * term->cols + x] = ' ';

	term_request_update(term);

	if ((term->cursor_x == 0) && (term->cursor_y == term->rows - 1)) {
		term->cursor_x = term->cols;
		term->cursor_y--;
	}

	if (term->cursor_x > 0)
		term->cursor_x--;

}

/* writes a given @size of bytes of @str string to primary buffer at current cursor position */
void term_puts(term_win *term, const char *str/*, const uint8_t n*/)
{
	term_goto(term, 0, 0);

	while (*str != '\0') {
		term_putch(term, *str);

		if (term->flags.v_overflow)
			break;

		str++;
	}
}

void term_putsl(term_win *term, const char *str, const uint8_t line)
{
	term_goto(term, 0, line);

	while (*str != '\0') {
		term_putch(term, *str);

		if (term->flags.h_overflow)
			break;

		str++;
	}
}

void term_putsxy(term_win *term, const char *str, const uint8_t x, const uint8_t y)
{
	term_goto(term, x, y);

	while (*str != '\0') {
		term_putch(term, *str);

		if (term->flags.h_overflow)
			break;

		str++;
	}
}

/* initializes std_win terminal */
void term_init(term_win *term)
{
	lcd_init(LCD_CFG_4BIT_2LINE_5X8,
			 LCD_CFG_DISPLAY1_BLINK0_CURSOR0,
			 LCD_CFG_INC_NOSHIFT
			);      // initialize lcd display

	term->cursor_x = 0;   // set default cursor position
	term->cursor_y = 0;

	term->size = TERM_2L_SIZE;
	term->rows = 2;
	term->cols = 16;

	term->flags.blink = false;   // force first update
	term->flags.cursor = false;     // not implemented yet
	term->flags.update_req = true;

	term_cls(term);     //flush the screen buffer
}
