/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 31.03.2020                                                  */
/************************************************************************/

#include "motor.h"

pwm_channel_t motorLeftPWM;
pwm_channel_t motorRightPWM;
pwm_channel_t motorRearPWM;

#define MAX_MOTOR_SPEED         (500) //PWM units

// local functions
void motorUpdatePWM(uint8_t motor, int16_t ispeed);

void motorInit(void)
{
    /* Initialize PWM channel for left motor */
    motorLeftPWM.alignment = PWM_ALIGN_LEFT;
    motorLeftPWM.polarity = PWM_LOW;
    motorLeftPWM.ul_prescaler = PWM_CMR_CPRE_CLKA;
    motorLeftPWM.ul_period = PERIOD_VALUE;
    motorLeftPWM.ul_duty = INIT_DUTY_VALUE;
    motorLeftPWM.channel = MOTOR_LEFT;
    pwm_channel_init(PWM, &motorLeftPWM);

    /* Initialize PWM channel for right motor */
    motorRightPWM.alignment = PWM_ALIGN_LEFT;
    motorRightPWM.polarity = PWM_LOW;
    motorRightPWM.ul_prescaler = PWM_CMR_CPRE_CLKA;
    motorRightPWM.ul_period = PERIOD_VALUE;
    motorRightPWM.ul_duty = INIT_DUTY_VALUE;
    motorRightPWM.channel = MOTOR_RIGHT;
    pwm_channel_init(PWM, &motorRightPWM);

    /* Initialize PWM channel for rear motor */
    motorRearPWM.alignment = PWM_ALIGN_LEFT;
    motorRearPWM.polarity = PWM_LOW;
    motorRearPWM.ul_prescaler = PWM_CMR_CPRE_CLKA;
    motorRearPWM.ul_period = PERIOD_VALUE;
    motorRearPWM.ul_duty = INIT_DUTY_VALUE;
    motorRearPWM.channel = MOTOR_REAR;
    pwm_channel_init(PWM, &motorRearPWM);
    
    motorDisable();
}

void motorEnable(void)
{
    pwm_channel_enable(PWM, MOTOR_LEFT);
    pwm_channel_enable(PWM, MOTOR_RIGHT);
    pwm_channel_enable(PWM, MOTOR_REAR);
}

void motorDisable(void)
{
    pwm_channel_disable(PWM, MOTOR_LEFT);
    pwm_channel_disable(PWM, MOTOR_RIGHT);
    pwm_channel_disable(PWM, MOTOR_REAR);
}

void moveRobot(move_t action, float speed, float rot)
{
    int16_t ispeed = (int16_t)(-speed * 5 + 0.5f);
    int16_t irot = (int16_t)(-rot * 5 + 0.5f);
    
    switch(action)
    {
        case MOVE_NONE:
            motorUpdatePWM(MOTOR_LEFT, irot);
            motorUpdatePWM(MOTOR_RIGHT, irot);
            motorUpdatePWM(MOTOR_REAR, irot);
            break;
        case MOVE_F:
            motorUpdatePWM(MOTOR_LEFT, -ispeed);
            motorUpdatePWM(MOTOR_RIGHT, ispeed);
            motorUpdatePWM(MOTOR_REAR, irot);
            break;
        case MOVE_FL:
            motorUpdatePWM(MOTOR_LEFT, irot);
            motorUpdatePWM(MOTOR_RIGHT, ispeed);
            motorUpdatePWM(MOTOR_REAR, -ispeed);
            break;
        case MOVE_FR:
            motorUpdatePWM(MOTOR_LEFT, -ispeed);
            motorUpdatePWM(MOTOR_RIGHT, irot);
            motorUpdatePWM(MOTOR_REAR, ispeed);
            break;
        case MOVE_B:
            motorUpdatePWM(MOTOR_LEFT, ispeed);
            motorUpdatePWM(MOTOR_RIGHT, -ispeed);
            motorUpdatePWM(MOTOR_REAR, irot);
            break;
        case MOVE_BL:
            motorUpdatePWM(MOTOR_LEFT, ispeed);
            motorUpdatePWM(MOTOR_RIGHT, irot);
            motorUpdatePWM(MOTOR_REAR, -ispeed);
            break;
        case MOVE_BR:
            motorUpdatePWM(MOTOR_LEFT, irot);
            motorUpdatePWM(MOTOR_RIGHT, -ispeed);
            motorUpdatePWM(MOTOR_REAR, ispeed);
            break;
        case MOVE_RL:
            motorUpdatePWM(MOTOR_LEFT, ispeed);
            motorUpdatePWM(MOTOR_RIGHT, ispeed);
            motorUpdatePWM(MOTOR_REAR, ispeed);
            break;
        case MOVE_RR:
            motorUpdatePWM(MOTOR_LEFT, -ispeed);
            motorUpdatePWM(MOTOR_RIGHT, -ispeed);
            motorUpdatePWM(MOTOR_REAR, -ispeed);
            break;
        default:
            break;
    }
}

void motorSetIndividual(uint8_t motor, float ispeed)
{
    int16_t jspeed = (int16_t)(-ispeed * 5 + 0.5f);
    
    /* compensate motor output */
    if(jspeed > MAX_MOTOR_SPEED) jspeed = MAX_MOTOR_SPEED;
    if(jspeed < -MAX_MOTOR_SPEED) jspeed = -MAX_MOTOR_SPEED;
    
    motorUpdatePWM(motor, jspeed);
}

void motorUpdatePWM(uint8_t motor, int16_t ispeed)
{
    if (ispeed > 500) ispeed = 500;
    if (ispeed < -500) ispeed = -500;

    uint16_t duty_cycle = (uint16_t)(ispeed + (int16_t)INIT_DUTY_VALUE);
    
    /* make PWM recognizable for motor driver */
    if (duty_cycle < 10) duty_cycle = 10;
    if (duty_cycle > 990) duty_cycle = 990;

    switch(motor)
    {
        case MOTOR_LEFT:
            pwm_channel_update_duty(PWM, &motorLeftPWM, duty_cycle);
            break;
        case MOTOR_RIGHT:
            pwm_channel_update_duty(PWM, &motorRightPWM, duty_cycle);
            break;
        case MOTOR_REAR:
            pwm_channel_update_duty(PWM, &motorRearPWM, duty_cycle);
            break;
        default:
            break;
    }
}