/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.03.2019                                                  */
/************************************************************************/

#include "motor.h"
#include "asf.h"

pwm_channel_t pwm_motor_left;
pwm_channel_t pwm_motor_right;
pwm_channel_t pwm_motor_rear;

void motor_init(void)
{
    /* Initialize PWM channel for left motor */
    pwm_motor_left.alignment = PWM_ALIGN_LEFT;
    pwm_motor_left.polarity = PWM_LOW;
    pwm_motor_left.ul_prescaler = PWM_CMR_CPRE_CLKA;
    pwm_motor_left.ul_period = PERIOD_VALUE;
    pwm_motor_left.ul_duty = INIT_DUTY_VALUE;
    pwm_motor_left.channel = MOTOR_LEFT;
    pwm_channel_init(PWM, &pwm_motor_left);

    /* Initialize PWM channel for right motor */
    pwm_motor_right.alignment = PWM_ALIGN_LEFT;
    pwm_motor_right.polarity = PWM_LOW;
    pwm_motor_right.ul_prescaler = PWM_CMR_CPRE_CLKA;
    pwm_motor_right.ul_period = PERIOD_VALUE;
    pwm_motor_right.ul_duty = INIT_DUTY_VALUE;
    pwm_motor_right.channel = MOTOR_RIGHT;
    pwm_channel_init(PWM, &pwm_motor_right);

    /* Initialize PWM channel for rear motor */
    pwm_motor_rear.alignment = PWM_ALIGN_LEFT;
    pwm_motor_rear.polarity = PWM_LOW;
    pwm_motor_rear.ul_prescaler = PWM_CMR_CPRE_CLKA;
    pwm_motor_rear.ul_period = PERIOD_VALUE;
    pwm_motor_rear.ul_duty = INIT_DUTY_VALUE;
    pwm_motor_rear.channel = MOTOR_REAR;
    pwm_channel_init(PWM, &pwm_motor_rear);
    
    disable_motor();
}

void enable_motor(void)
{
    pwm_channel_enable(PWM, MOTOR_LEFT);
    pwm_channel_enable(PWM, MOTOR_RIGHT);
    pwm_channel_enable(PWM, MOTOR_REAR);
}

void disable_motor(void)
{
    pwm_channel_disable(PWM, MOTOR_LEFT);
    pwm_channel_disable(PWM, MOTOR_RIGHT);
    pwm_channel_disable(PWM, MOTOR_REAR);
}

void move_robot(move_t action, float speed, float rot)
{
    int16_t ispeed = (int16_t)(speed * 5 + 0.5f);
    int16_t irot = (int16_t)(rot * 5 + 0.5f);
    
    switch(action)
    {
        case MOVE_NONE:
            update_motor_pwm(MOTOR_LEFT, irot);
            update_motor_pwm(MOTOR_RIGHT, irot);
            update_motor_pwm(MOTOR_REAR, irot);
			break;
        case MOVE_F:
            update_motor_pwm(MOTOR_LEFT, -ispeed);
            update_motor_pwm(MOTOR_RIGHT, ispeed);
            update_motor_pwm(MOTOR_REAR, irot);
            break;
        case MOVE_FL:
            update_motor_pwm(MOTOR_LEFT, irot);
            update_motor_pwm(MOTOR_RIGHT, ispeed);
            update_motor_pwm(MOTOR_REAR, -ispeed);
            break;
        case MOVE_FR:
            update_motor_pwm(MOTOR_LEFT, -ispeed);
            update_motor_pwm(MOTOR_RIGHT, irot);
            update_motor_pwm(MOTOR_REAR, ispeed);
            break;
        case MOVE_B:
            update_motor_pwm(MOTOR_LEFT, ispeed);
            update_motor_pwm(MOTOR_RIGHT, -ispeed);
            update_motor_pwm(MOTOR_REAR, irot);
            break;
        case MOVE_BL:
            update_motor_pwm(MOTOR_LEFT, ispeed);
            update_motor_pwm(MOTOR_RIGHT, irot);
            update_motor_pwm(MOTOR_REAR, -ispeed);
            break;
        case MOVE_BR:
            update_motor_pwm(MOTOR_LEFT, irot);
            update_motor_pwm(MOTOR_RIGHT, -ispeed);
            update_motor_pwm(MOTOR_REAR, ispeed);
            break;
        case MOVE_RL:
            update_motor_pwm(MOTOR_LEFT, ispeed);
            update_motor_pwm(MOTOR_RIGHT, ispeed);
            update_motor_pwm(MOTOR_REAR, ispeed);
            break;
        case MOVE_RR:
            update_motor_pwm(MOTOR_LEFT, -ispeed);
            update_motor_pwm(MOTOR_RIGHT, -ispeed);
            update_motor_pwm(MOTOR_REAR, -ispeed);
            break;
        default:
            break;
    }
}

void set_motor_individual(uint8_t motor, float ispeed)
{
    int16_t jspeed = (int16_t)(ispeed * 5 + 0.5f);
    /* compensate motor output */
    if(jspeed > MAX_MOTOR_SPEED) jspeed = MAX_MOTOR_SPEED;
    if(jspeed < -MAX_MOTOR_SPEED) jspeed = -MAX_MOTOR_SPEED;
    
    update_motor_pwm(motor, jspeed);
}

void update_motor_pwm(uint8_t motor, int16_t ispeed)
{
    if (ispeed > 500) ispeed = 500;
    if (ispeed < -500) ispeed = -500;

    uint16_t duty_cycle = (uint16_t)(ispeed + (int16_t)INIT_DUTY_VALUE);

    if (duty_cycle < 10) duty_cycle = 10;
    if (duty_cycle > 990) duty_cycle = 990;

    switch(motor)
    {
        case MOTOR_LEFT:
            pwm_channel_update_duty(PWM, &pwm_motor_left, duty_cycle);
            break;
        case MOTOR_RIGHT:
            pwm_channel_update_duty(PWM, &pwm_motor_right, duty_cycle);
            break;
        case MOTOR_REAR:
            pwm_channel_update_duty(PWM, &pwm_motor_rear, duty_cycle);
            break;
        default:
            break;
    }
}
