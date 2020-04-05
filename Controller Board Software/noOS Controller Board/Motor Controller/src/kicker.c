/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 04.04.2020                                                  */
/************************************************************************/

#include "kicker.h"
#include "data.h"
#include "timing.h"

uint32_t lastKick = 0;
uint32_t ticks_vkick = 0;

// local functions
void updateVkick(void);

void kickerMaintenance(void)
{
    updateVkick();
    
    if((getTicks() - lastKick) >= 100) // reset kicker to start charging
    {
        ioport_set_pin_level(KICK_TRIG, 0);
    }
}

void kick(void)
{
    if(data.kickerPercentage >= 70 && (getTicks() - lastKick) >= 100)
    {
        lastKick = getTicks();
        ioport_set_pin_level(KICK_TRIG, 1);
    }
}

void updateVkick(void)
{
    if((getTicks() - ticks_vkick) >= 200)
    {
        ticks_vkick = getTicks();
        
        uint32_t rawVkick = adc_get_channel_value(ADC, KICK_VOLTAGE);
        data.kickerPercentage = (rawVkick / 37.0f);
        adc_start(ADC);
    }
}
