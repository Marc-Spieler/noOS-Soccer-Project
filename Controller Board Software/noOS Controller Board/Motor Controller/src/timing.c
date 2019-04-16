/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: NoOS                                                           */
/* Created: 01.07.18                                                    */
/************************************************************************/

#include "timing.h"
#include "comm.h"
#include "string.h"
#include "lcd.h"
#include "menu.h"

uint32_t ul_ticks_compass;

static uint32_t g_ul_ms_ticks = 0;
//float battery_voltage = 0;
uint8_t battery_percentage;
uint8_t prev_battery_percentage = 0;

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

void update_comm(void)
{
    if ((getTicks() - ticks_comm) > 5)
    {
        ticks_comm = getTicks();
        
        memcpy(&sens_buf, &mts, sizeof(mts));
        spi_master_transfer(&sens_buf, sizeof(sens_buf));
    }
}

void update_battery(Bool update_forced)
{
    char tmp[6];
    battery_percentage = stm.battery.percentage;
    
    if (update_forced || prev_battery_percentage != battery_percentage)
    {
        if (battery_percentage >= 100)
        {
            sprintf(tmp, "%3u%%", battery_percentage);
        }
        else if (battery_percentage >= 10)
        {
            sprintf(tmp, " %2u%%", battery_percentage);
        }
        else
        {
            sprintf(tmp, "  %1u%%", battery_percentage);
        }
            
        lcd_print_s(1, 16, tmp);
        prev_battery_percentage = battery_percentage;
    }
}

void update_heartbeat(void)
{
    if (heart_state)
    {
        if (getTicks() >= (ticks_heartbeat + 100))
        {
            ticks_heartbeat = getTicks();
            ioport_set_pin_level(LED_ONBOARD, 0);
            ioport_set_pin_level(LED_M1, 0);
            mts.ibit.heartbeat = 0;
            heart_state = 0;
        }
    }
    else
    {
        if (getTicks() >= (ticks_heartbeat + 900))
        {
            ticks_heartbeat = getTicks();
            ioport_set_pin_level(LED_ONBOARD, 1);
            ioport_set_pin_level(LED_M1, 1);
            mts.ibit.heartbeat = 1;
            heart_state = 1;
        }
    }
}
