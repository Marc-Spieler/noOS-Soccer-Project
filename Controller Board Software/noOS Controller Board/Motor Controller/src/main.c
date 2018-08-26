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

backlight_t bl_state = LCD_LIGHT_OFF;

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

int main(void)
{
    event_t act_event;

    sysclk_init();
    board_init();
    SysTick_Config(sysclk_get_cpu_hz() / 1000);
    
    //sd_mmc_init();
    
    spi_init();
    
    lcd_init();
    bl_state = LCD_LIGHT_ON;
    lcd_set_backlight(bl_state);
    
/*//    while (1)
//    {
        //printf("Please plug an SD, MMC or SDIO card in slot.\n\r");
        do
        {
            status = sd_mmc_test_unit_ready(0);
            
            if (CTRL_FAIL == status)
            {
                //printf("Card install FAIL\n\r");
                //printf("Please unplug and re-plug the card.\n\r");
                while (CTRL_NO_PRESENT != sd_mmc_check(0));
            }
        } while (CTRL_GOOD != status);

        //printf("Mount disk (f_mount)...\r\n");
        memset(&fs, 0, sizeof(FATFS));
        res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);
        
        if (FR_INVALID_DRIVE == res)
        {
            //printf("[FAIL] res %d\r\n", res);
            goto main_end_of_test;
        }
        //printf("[OK]\r\n");

        //printf("Create a file (f_open)...\r\n");
        test_file_name[0] = LUN_ID_SD_MMC_0_MEM + '0';
        res = f_open(&file_object, (char const *)test_file_name, FA_CREATE_ALWAYS | FA_WRITE);
        
        if (res != FR_OK)
        {
            //printf("[FAIL] res %d\r\n", res);
            goto main_end_of_test;
        }
        
//        if (0 == f_puts("Test SD/MMC stack\n", &file_object))
//        {
//            f_close(&file_object);
//            //printf("[FAIL]\r\n");
//            goto main_end_of_test;
//        }
        
        //printf("[OK]\r\n");
        f_close(&file_object);
        //printf("Test is successful.\n\r");

main_end_of_test:
        //printf("Please unplug the card.\n\r");
        //while (CTRL_NO_PRESENT != sd_mmc_check(0));
//    }*/
    
    noOS_bootup_sequence();
    
    lcd_clear();
    
    while (1)
    {
        update_comm();
        update_heartbeat();
        update_battery(0);
        
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
        
        if (act_event == EVENT_BUTTON_MID_H)
        {
            mts.ibit.button = 1;
        }
        else
        {
            mts.ibit.button = 0;
        }  
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
