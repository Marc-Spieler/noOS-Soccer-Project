/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.03.2019                                                  */
/************************************************************************/

#ifndef MOTOR_H
#define MOTOR_H

#include "asf.h"
#include "pid.h"

#define MAX_MOTOR_SPEED         (150) //cm/s

extern pidReg_t pid_motor_left;
extern pidReg_t pid_motor_right;
extern pidReg_t pid_motor_rear;
extern int16_t opponent_goal;
extern int16_t own_goal;

extern float act_motor_speed_left;
extern float act_motor_speed_right;
extern float act_motor_speed_rear;

void motor_init(void);
void enable_motor(void);
void disable_motor(void);

void set_motor(float speed, float dir, float trn);
void set_motor_extended(float speed, float dir, float trn, float left, float right, float rear);
void set_motor_individual(float mleft_ref, float mright_ref, float mrear_ref);
void update_motor_pwm(uint8_t motor, int16_t speed);

#endif
