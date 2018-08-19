/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 29.01.18                                                    */
/************************************************************************/

#ifndef LDC_H
#define LDC_H

#include "asf.h"

typedef enum bl_type
{
	LCD_LIGHT_OFF = 0,
	LCD_LIGHT_ON  = 8
} backlight_t;

void lcd_init(void);
void set_backlight(backlight_t state);
void lcd_clear(void);
void print_s(uint8_t line, uint8_t col, const char* data);
void lcd_print_c(int data);
void print_c(uint8_t line, uint8_t col, int data);
void twi_init(void);
//void update_lcd(void);

#endif