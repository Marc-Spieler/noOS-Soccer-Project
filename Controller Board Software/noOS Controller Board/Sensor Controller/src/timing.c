/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.08.18                                                    */
/************************************************************************/

#include "timing.h"
#include "comm.h"
#include "math.h"

static uint32_t g_ul_ms_ticks = 0;
uint32_t ul_ticks_bat = 0;
float adc_bat_value;
uint8_t bat_percentage_prefiltered = 0;
uint8_t bat_percentage_filtered = 0;

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
    if ((getTicks() - ul_ticks_bat) >= 500)
    {
        ul_ticks_bat = getTicks();
        
        while ((adc_get_status(ADC) & ADC_ISR_DRDY) != ADC_ISR_DRDY);
        
        adc_bat_value = (float)(adc_get_channel_value(ADC, BATTERY_VOLTAGE));
        //stm.bat_voltage = (uint8_t)(adc_bat_value / 18.0f);
        float y = adc_bat_value * 0.003472222;
        bat_percentage_prefiltered = (uint8_t)(-(y * y - 25.2f * y + 158.76f) / 0.04f + 100.0f);
        bat_percentage_prefiltered = (bat_percentage_prefiltered - 20.0) * 1.25;
        bat_percentage_filtered = (bat_percentage_filtered * 3 + bat_percentage_prefiltered) / 4;
        stm.bat_percentage = bat_percentage_filtered;
        adc_start(ADC);
    }
}

void update_heartbeat(void)
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
}    
