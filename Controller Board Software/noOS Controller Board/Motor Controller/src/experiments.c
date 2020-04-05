/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 04.04.2020                                                  */
/************************************************************************/

#include "experiments.h"
#include "data.h"
#include "motor.h"
#include "timing.h"

Bool doTest = false;
float pidP = 2.0f;

uint32_t lastDirectionChange = 0;
uint8_t movementDirection = 0;

void motorCircleTest(void)
{
    if((getTicks() - lastDirectionChange) >= 800 && doTest)
    {
        lastDirectionChange = getTicks();
        motorEnable();
        
        switch(movementDirection)
        {
            case 0:
                movementDirection = MOVE_F;
                break;
            case MOVE_F:
                movementDirection = MOVE_FR;
                break;
            case MOVE_FR:
                movementDirection = MOVE_BR;
                break;
            case MOVE_BR:
                movementDirection = MOVE_B;
                break;
            case MOVE_B:
                movementDirection = MOVE_BL;
                break;
            case MOVE_BL:
                movementDirection = MOVE_FL;
                break;
            case MOVE_FL:
                movementDirection = 0;
                motorDisable();
                doTest = false;
                break;
            default:
                break;
        }
    }
	motorEnable();
	moveRobot(movementDirection, 0, (data.compass * pidP));
}    