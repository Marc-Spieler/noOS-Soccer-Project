/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.07.18                                                    */
/************************************************************************/

#include "asf.h"
#include "string.h"
#include "timing.h"
#include "lcd.h"
#include "menu.h"
#include "comm.h"
#include "compass.h"
#include "motor.h"

Bool blink_level;
uint32_t ticks_blink_update;
uint32_t ticks_dot_update;
uint8_t dots = 0;
Bool update_dots = 1;

/*char test_file_name[] = "0:sd_mmc_test.txt";
Ctrl_status status;
FRESULT res;
FATFS fs;
FIL file_object;*/

void noOS_bootup_sequence(void);
void set_led(ioport_pin_t pin, Bool level);

int main(void)
{
    event_t act_event;

    sysclk_init();
    board_init();
    SysTick_Config(sysclk_get_cpu_hz() / 1000);
    
    motor_init();
    
    sd_mmc_init();
    
    spi_init();
    
    compass_init();
    lcd_init();
    
	/*sd_mmc_err_t err;
	while (1)
	{
    	do
    	{
        	status = sd_mmc_test_unit_ready(0);
        	
        	if (CTRL_FAIL == status)
        	{
            	while (CTRL_NO_PRESENT != sd_mmc_check(0));
        	}
    	}
    	while (CTRL_GOOD != status);

    	memset(&fs, 0, sizeof(FATFS));
    	res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);
    	
    	if (FR_INVALID_DRIVE == res)
    	{
        	while(1);
    	}

    	test_file_name[0] = LUN_ID_SD_MMC_0_MEM + '0';
    	res = f_open(&file_object, (char const *)test_file_name, FA_CREATE_ALWAYS | FA_WRITE);
    	
    	if (res != FR_OK)
    	{
        	while(1);
    	}

#if 1
        uint32_t bw;
        uint32_t sta = getTicks();
        set_led(LED_ONBOARD, 0);
    	for(int i = 0; i < 10000; i++)
        {
        	f_write(&file_object, "Test SD/MMC stack\n", 18, &bw);

        }
        uint32_t diff = getTicks() - sta;
  	    f_close(&file_object);
        set_led(LED_ONBOARD, 1);
#else
        uint32_t bw;
    	if (0 == f_write(&file_object, "Test SD/MMC stack\n", 18, &bw))
    	{
    	    f_close(&file_object);
    	}
        else
        {
        	f_close(&file_object);
        }
#endif
    	while (CTRL_NO_PRESENT != sd_mmc_check(0));
	}*/

    /*memset(&fs, 0, sizeof(FATFS));
    res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);
    test_file_name[0] = LUN_ID_SD_MMC_0_MEM + '0';
    res = f_open(&file_object, (char const *)test_file_name, FA_CREATE_ALWAYS | FA_WRITE);
    f_write(&file_object, "Test SD/MMC stack\n", 18, &bw);
    f_close(&file_object);*/
    
    noOS_bootup_sequence();
    
    while (1)
    {
        update_comm();
        update_heartbeat();
        //check_battery();
        
        if (stm.ibit.heartbeat)
        {
            set_led(LED_M2, 1);
        }
        else
        {
            set_led(LED_M2, 0);
        }
        
        update_motor(5, 5, 5);
        
        act_event = button_events();
        
        menu(act_event);
    }
}

void noOS_bootup_sequence(void)
{
    while (!ioport_get_pin_level(RPI2) && ioport_get_pin_level(PB_MID))
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
                lcd_print_s(2, 2, "booting noOS   ");
                break;
                case 1:
                lcd_print_s(2, 14, ".");
                break;
                case 2:
                lcd_print_s(2, 15, ".");
                break;
                case 3:
                lcd_print_s(2, 16, ".");
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

void set_led(ioport_pin_t pin, Bool level)
{
    if (allow_leds)
    {
        ioport_set_pin_level(pin, level);
    }
    else
    {
        ioport_set_pin_level(pin, 0);
    }
}
