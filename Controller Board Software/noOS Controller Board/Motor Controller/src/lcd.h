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
void lcd_set_backlight(backlight_t state);
void lcd_clear(void);
void lcd_print_i(int8_t line, uint8_t col, uint32_t data);
void lcd_print_s(int8_t line, uint8_t col, const char* str);
Bool lcd_is_busy(void);
uint32_t lcd_get_timeout_error_cntr(void);

#endif