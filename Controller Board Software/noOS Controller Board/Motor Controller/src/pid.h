/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.03.2019                                                  */
/************************************************************************/

#ifndef PID_REG_H
#define PID_REG_H

typedef struct
{
    float kp;       // Parameter: Proportional gain
    float ki;       // Parameter: Integral gain
    float kc;       // Parameter: Integral correction gain
    float kd;       // Parameter: Derivative gain
    float intg;     // Variable:  Integral output
    float outMax;   // Parameter: Maximum output
    float outMin;   // Parameter: Minimum output
    float satErr;   // Variable:  Saturated difference
    float prevErr;  // History:   Previous error
} pidReg_t;

float pidReg(pidReg_t* reg, float ref, float act);
float pidReg_compass(pidReg_t* reg, float ref, float act);

#endif
