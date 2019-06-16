/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.07.2018                                                  */
/************************************************************************/

#include "asf.h"
#include "string.h"
#include "timing.h"
#include "lcd.h"
#include "menu.h"
#include "comm.h"
#include "compass.h"
#include "sd.h"
#include "motor.h"
#include "iniparser.h"
#include "support.h"
#include "math.h"

uint8_t trn = 5;

void noOS_bootup_sequence(void);

int main(void)
{
    event_t act_event;

    sysclk_init();

    board_init();
    SysTick_Config(sysclk_get_cpu_hz() / 1000);
    
    motor_init();
    //init_battery_warning();

    compass_init();
    lcd_init();

    sd_mmc_init();
    sd_init();
    //write_time_test_2();
    //create_default_ini_file();
    parse_ini_file();
    spi_init();

    while (1)
    {
        update_comm();
        update_compass();
        update_heartbeat();
        check_battery();
        
        prepare_values_to_send();
        process_new_sensor_values();

        act_event = button_events();
        menu(act_event);
    }
}
