/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 29.01.18                                                    */
/************************************************************************/

#ifndef LCD_H
#define LCD_H

#include "asf.h"

typedef enum bl_type
{
    LCD_LIGHT_OFF = 0,
    LCD_LIGHT_ON  = 8
} backlight_t;

extern backlight_t bl_state;

void lcd_init(void);
void lcd_set_backlight(backlight_t state);
void lcd_clear(void);
void lcd_print_i(uint8_t line, uint8_t col, int32_t data);
void lcd_print_s(uint8_t line, uint8_t col, const char* str);
void lcd_print_m(const char* str[]);
uint8_t lcd_is_busy(void);
uint32_t lcd_get_timeout_error_cntr(void);

#endif
