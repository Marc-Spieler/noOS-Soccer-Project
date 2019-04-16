/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.07.2018                                                  */
/************************************************************************/

#include "asf.h"
#include "comm.h"
#include "timing.h"
#include "line.h"

int main(void)
{
    sysclk_init();
    board_init();
    SysTick_Config(sysclk_get_cpu_hz() / 1000);
    
    spi_init();
    dacc_init();
    adc_start(ADC);
    
    for(int i = 0; i< 3; i++)
    {
        set_led(LED_ONBOARD, 1);
        set_led(LED_S1, 1);
        set_led(LED_S2, 1);
        set_led(LED_S3, 1);
        mdelay(100);
        set_led(LED_ONBOARD, 0);
        set_led(LED_S1, 0);
        set_led(LED_S2, 0);
        set_led(LED_S3, 0);
        mdelay(100);
    }
    
    while (1)
    {
        update_battery();
        update_heartbeat();
        
        update_line_values();
        calculate_line_esc_direction();
        update_line_calibration_value(mts.line_cal_value * 256);
        
        PrepareValuesToSend();
    }
}

