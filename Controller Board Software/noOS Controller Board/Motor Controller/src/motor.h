/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.03.2019                                                  */
/************************************************************************/

#ifndef MOTOR_H
#define MOTOR_H

#include "asf.h"

extern int16_t opponent_goal;
extern int16_t own_goal;
//extern int16_t rel_deviation;
extern float speed;
extern float mleft;
extern float mright;
extern float mrear;
extern float SinMA1;
extern float SinMA2;
extern float SinMA3;
extern float CosinMA1;
extern float CosinMA2;
extern float CosinMA3;

void motor_init(void);
void enable_motor(void);
void disable_motor(void);
void update_motor(float mleft_ref, float mright_ref, float mrear_ref);
void motor_speed(uint8_t motor, int16_t speed);
/*void set_opponent_goal(void);
void set_inv_opponent_goal(void);
void set_own_goal(void);
int16_t estimate_rel_deviation(uint16_t dir, int16_t tar);*/

#endif