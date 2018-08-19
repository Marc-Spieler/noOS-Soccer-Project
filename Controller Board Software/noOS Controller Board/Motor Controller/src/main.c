/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.07.18                                                    */
/************************************************************************/

#include "asf.h"
#include "timing.h"
#include "lcd.h"
#include "menu.h"
#include "comm.h"

backlight_t bl_state = LCD_LIGHT_OFF;

Bool blink_level;
uint32_t ticks_blink_update;
uint32_t ticks_dot_update;
uint8_t dots = 0;
Bool update_dots = 1;

void noOS_bootup_sequence(void);

int main(void)
{
	event_t act_event;

	sysclk_init();
	board_init();
	SysTick_Config(sysclk_get_cpu_hz() / 1000);
	
	spi_init();
	
	twi_init();
	
	lcd_init();
	bl_state = LCD_LIGHT_ON;
	set_backlight(bl_state);
	
	noOS_bootup_sequence();
	
	lcd_clear();
	
	while (1)
	{
		update_comm();
		update_heartbeat();
		
		if (stm.ibit.heartbeat)
		{
			ioport_set_pin_level(LED_M2, 1);
		}
		else
		{
			ioport_set_pin_level(LED_M2, 0);
		}
		
		act_event = button_events();
		
		menu(act_event);
	}
}

void noOS_bootup_sequence(void)
{
	while (ioport_get_pin_level(PB_MID))
	{
		if (getTicks() >= (ticks_blink_update + 800))
		{
			ticks_blink_update = getTicks();
			
			if (blink_level)
			{
				blink_level = 0;
			}
			else
			{
				blink_level = 1;
			}
			
			ioport_set_pin_level(LED_BAT, blink_level);
		}
		
		if (getTicks() >= (ticks_dot_update + 500))
		{
			ticks_dot_update = getTicks();
			
			if (dots < 3)
			{
				dots++;
			}
			else
			{
				dots = 0;
			}
			
			update_dots = 1;
		}
		
		if (update_dots)
		{
			update_dots = 0;
			
			switch (dots)
			{
				case 0:
				print_s(2, 2, "booting noOS   ");
				break;
				case 1:
				print_s(2, 14, ".");
				break;
				case 2:
				print_s(2, 15, ".");
				break;
				case 3:
				print_s(2, 16, ".");
				break;
				default:
				break;
			}
		}
	}
	
	for(int i = 0; i< 3; i++)
	{
		ioport_set_pin_level(LED_ONBOARD, 1);
		ioport_set_pin_level(LED_BAT, 1);
		ioport_set_pin_level(LED_M1, 1);
		ioport_set_pin_level(LED_M2, 1);
		ioport_set_pin_level(LED_M3, 1);
		mdelay(100);
		ioport_set_pin_level(LED_ONBOARD, 0);
		ioport_set_pin_level(LED_BAT, 0);
		ioport_set_pin_level(LED_M1, 0);
		ioport_set_pin_level(LED_M2, 0);
		ioport_set_pin_level(LED_M3, 0);
		mdelay(100);
	}
}
