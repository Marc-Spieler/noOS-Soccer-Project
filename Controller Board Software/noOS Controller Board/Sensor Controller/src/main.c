/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.07.18                                                    */
/************************************************************************/

#include "asf.h"
#include "comm.h"
#include "timing.h"

int main(void)
{
    sysclk_init();
    board_init();
    SysTick_Config(sysclk_get_cpu_hz() / 1000);
    
    spi_init();
    
    adc_start(ADC);
    
    for(int i = 0; i< 3; i++)
    {
        ioport_set_pin_level(LED_ONBOARD, 1);
        ioport_set_pin_level(LED_S1, 1);
        ioport_set_pin_level(LED_S2, 1);
        ioport_set_pin_level(LED_S3, 1);
        mdelay(100);
        ioport_set_pin_level(LED_ONBOARD, 0);
        ioport_set_pin_level(LED_S1, 0);
        ioport_set_pin_level(LED_S2, 0);
        ioport_set_pin_level(LED_S3, 0);
        mdelay(100);
    }
    
    while (1)
    {
        update_battery();
        update_heartbeat();
        
        
        PrepareValuesToSend();
    }
}
