/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 29.03.2020                                                  */
/************************************************************************/

#include "asf.h"
#include "compass.h"
#include "experiments.h"
#include "kicker.h"
#include "motor.h"
#include "ric.h"
#include "serial.h"
#include "timing.h"

int main(void)
{
    sysclk_init();
    boardInit();
    SysTick_Config(sysclk_get_cpu_hz() / 1000);
    
    serialInit();
    ricInit();
    
    compassInit();
    
    motorInit();
    
    while(1)
    {
        compassMaintenance();
        kickerMaintenance();
        ricMaintenance();
        serialMaintenance();
        
        motorCircleTest();
    }
}
