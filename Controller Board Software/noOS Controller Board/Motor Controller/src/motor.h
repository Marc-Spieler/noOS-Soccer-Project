/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 31.03.2020                                                  */
/************************************************************************/

#ifndef MOTOR_H
#define MOTOR_H

#include "asf.h"

typedef enum
{
    MOVE_NONE,
    MOVE_F,
    MOVE_FL,
    MOVE_FR,
    MOVE_B,
    MOVE_BL,
    MOVE_BR,
    MOVE_RL,
    MOVE_RR,
} move_t;

void motorInit(void);
void motorEnable(void);
void motorDisable(void);
void moveRobot(move_t action, float speed, float rot);
void motorSetIndividual(uint8_t motor, float ispeed);

#endif