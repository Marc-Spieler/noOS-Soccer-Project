/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.03.2019                                                  */
/************************************************************************/

#ifndef MOTOR_H
#define MOTOR_H

#include "asf.h"

#define MAX_MOTOR_SPEED         (150) // cm/s

void motor_init(void);
void enable_motor(void);
void disable_motor(void);
void set_motor(int16_t x_speed, int16_t y_speed, int16_t trn);
void set_motor_individual(int16_t left, int16_t right, int16_t rear);

#endif
