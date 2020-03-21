/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.03.2019                                                  */
/************************************************************************/

#ifndef MOTOR_H
#define MOTOR_H

#include "asf.h"

#define MAX_MOTOR_SPEED         (500) //PWM unit

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

void motor_init(void);
void enable_motor(void);
void disable_motor(void);
void move_robot(move_t action, float speed, float rot);
void set_motor_individual(uint8_t motor, float ispeed);
void update_motor_pwm(uint8_t motor, int16_t speed);

#endif
