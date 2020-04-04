/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 29.03.2020                                                  */
/************************************************************************/

#include "timing.h"

static uint32_t msTicks = 0;

void SysTick_Handler(void)
{
    msTicks++;
}

uint32_t getTicks(void)
{
    return msTicks;
}

void mdelay(uint32_t delayDuration)
{
    uint32_t startTicks = msTicks;
    
    while ((msTicks - startTicks) < delayDuration);
}
