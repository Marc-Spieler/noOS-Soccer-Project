/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.07.18                                                    */
/************************************************************************/

#include "asf.h"
#include "comm.h"

static uint32_t g_ul_ms_ticks = 0;

void mdelay(uint32_t ul_dly_ticks);

void SysTick_Handler(void)
{
    g_ul_ms_ticks++;
}

int main(void)
{
    sysclk_init();
    board_init();
    SysTick_Config(sysclk_get_cpu_hz() / 1000);
    
    spi_init();
    
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
        if (mts.ibit.heartbeat)
        {
            ioport_set_pin_level(LED_ONBOARD, 1);
            ioport_set_pin_level(LED_S1, 1);
            stm.ibit.heartbeat = 1;
        }
        else
        {
            ioport_set_pin_level(LED_ONBOARD, 0);
            ioport_set_pin_level(LED_S1, 0);
            stm.ibit.heartbeat = 0;
        }
        
        if (mts.ibit.button)
        {
            ioport_set_pin_level(LED_S2, 1);
        }
        else
        {
            ioport_set_pin_level(LED_S2, 0);
        }
        
        PrepareValuesToSend();
    }
}

void mdelay(uint32_t ul_dly_ticks)
{
    uint32_t ul_cur_ticks;

    ul_cur_ticks = g_ul_ms_ticks;
    while ((g_ul_ms_ticks - ul_cur_ticks) < ul_dly_ticks);
}