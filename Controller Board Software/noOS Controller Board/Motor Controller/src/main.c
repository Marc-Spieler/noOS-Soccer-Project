/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.07.2018                                                  */
/************************************************************************/

#include "asf.h"
#include "string.h"
#include "timing.h"
#include "comm.h"
#include "compass.h"
#include "sd.h"
#include "motor.h"
#include "iniparser.h"
#include "support.h"
#include "bt.h"

int main(void)
{
    sysclk_init();

    board_init();
    SysTick_Config(sysclk_get_cpu_hz() / 1000);
    
    motor_init();

    compass_init();

    bt_init();
    
    sd_mmc_init();
    sd_init();
    parse_ini_file();
    
    spi_init();
    
    while (1)
    {
        update_comm();
        update_compass();
        update_heartbeat();
        check_battery();
		
		bt_maintenance();
        kicker_maintenance();
        
        prepare_values_to_send();
        process_new_sensor_values();

        if(matchStarted) match);
    }
}
