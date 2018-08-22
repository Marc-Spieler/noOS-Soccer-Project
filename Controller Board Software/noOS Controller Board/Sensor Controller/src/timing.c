/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.08.18                                                    */
/************************************************************************/

#include "timing.h"
#include "comm.h"

static uint32_t g_ul_ms_ticks = 0;
uint32_t ul_ticks_bat = 0;

void SysTick_Handler(void)
{
    g_ul_ms_ticks++;
}

uint32_t getTicks(void)
{
    return g_ul_ms_ticks;
}

void mdelay(uint32_t ul_dly_ticks)
{
    uint32_t ul_cur_ticks;

    ul_cur_ticks = g_ul_ms_ticks;
    while ((g_ul_ms_ticks - ul_cur_ticks) < ul_dly_ticks);
}

void update_battery(void)
{
    if ((getTicks() - ul_ticks_bat) > 500)
    {
        ul_ticks_bat = getTicks();
        
        while ((adc_get_status(ADC) & ADC_ISR_DRDY) != ADC_ISR_DRDY);
        
        stm.bat_voltage = (uint8_t)(adc_get_channel_value(ADC, BATTERY_VOLTAGE) / 18);
        adc_start(ADC);
    }
}
