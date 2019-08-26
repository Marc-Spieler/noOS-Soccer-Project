/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.03.2019                                                  */
/************************************************************************/

#include "motor.h"
#include "asf.h"
#include "string.h"
#include "math.h"
#include "sensor.h"

#define CM_PER_TICK             ((2 * M_PI * 3) / 464.64)
#define ENCODER_UPDATE_RATE     (0.008) //in seconds

pwm_channel_t pwm_motor_left;
pwm_channel_t pwm_motor_right;
pwm_channel_t pwm_motor_rear;
pwm_channel_t pwm_encoder;    // do not move - weird thing happens

int16_t motor_left_out;
int16_t motor_right_out;
int16_t motor_rear_out;

void update_motor_pwm(uint8_t motor, int16_t speed);

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

    pwm_channel_disable(PWM, MOTOR_LEFT);
    pwm_channel_disable(PWM, MOTOR_RIGHT);
    pwm_channel_disable(PWM, MOTOR_REAR);

    /* Initialize PWM channel for encoders */
    pwm_encoder.alignment = PWM_ALIGN_LEFT;
    pwm_encoder.polarity = PWM_LOW;
    pwm_encoder.ul_prescaler = PWM_CMR_CPRE_CLKA;
    pwm_encoder.ul_period = 330;
    pwm_encoder.ul_duty = 165;
    pwm_encoder.channel = ENC_CLK;
    pwm_channel_init(PWM, &pwm_encoder);
    pwm_channel_enable(PWM, ENC_CLK);
 
    sysclk_enable_peripheral_clock(ID_TC1);
    tc_init(TC0, 1, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_CPCTRG);
    tc_write_rc(TC0, 1, 5249);  //MCLK / 128 * 0,008
    NVIC_DisableIRQ(TC1_IRQn);
    NVIC_ClearPendingIRQ(TC1_IRQn);
    NVIC_SetPriority(TC1_IRQn, 0);
    NVIC_EnableIRQ(TC1_IRQn);
    tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
    tc_start(TC0, 1);
}

void enable_motor(void)
{
    pwm_channel_enable(PWM, MOTOR_LEFT);
    pwm_channel_enable(PWM, MOTOR_RIGHT);
    pwm_channel_enable(PWM, MOTOR_REAR);

    ioport_set_pin_level(ENC_LOAD, 0);
    ioport_set_pin_level(ENC_LOAD, 1);

    tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
}

void disable_motor(void)
{
    pwm_channel_disable(PWM, MOTOR_LEFT);
    pwm_channel_disable(PWM, MOTOR_RIGHT);
    pwm_channel_disable(PWM, MOTOR_REAR);

    tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
    s.motor.left_speed = 0.0f;
    s.motor.right_speed = 0.0f;
    s.motor.rear_speed = 0.0f;
}

void set_motor(int16_t x_speed, int16_t y_speed, int16_t trn)
{
    float left = -y_speed - x_speed * 0.5f + trn;
    float right = y_speed - x_speed * 0.5f + trn;
    float rear = x_speed + trn;
    
    /* compensate motor output */
    float motor[3] = {left, right, rear};

    if(motor[0] > MAX_MOTOR_SPEED || motor[1] > MAX_MOTOR_SPEED || motor[2] > MAX_MOTOR_SPEED)
    {
        uint8_t highest_value = 0;
        uint8_t highest_motor = 0;

        for(int i = 0; i < 3; i++)
        {
            if(motor[i] > highest_value)
            {
                highest_value = motor[i];
                highest_motor = i;
            }
        }

        float factor = motor[highest_motor] / MAX_MOTOR_SPEED;
        left /= factor;
        right /= factor;
        rear /= factor;
    }
    
    /* write new values to motors */
    update_motor_pwm(MOTOR_LEFT, left);
    update_motor_pwm(MOTOR_RIGHT, right);
    update_motor_pwm(MOTOR_REAR, rear);
}

void set_motor_individual(int16_t left, int16_t right, int16_t rear)
{
    /* write new values to motors */
    update_motor_pwm(MOTOR_LEFT, left);
    update_motor_pwm(MOTOR_RIGHT, right);
    update_motor_pwm(MOTOR_REAR, rear);
}

void update_motor_pwm(uint8_t motor, int16_t speed)
{
    speed = (speed > 500) ? 500 : speed;
    speed = (speed < -500) ? -500 : speed;

    uint16_t duty_cycle = (uint16_t)(speed + (int16_t)INIT_DUTY_VALUE);

    duty_cycle = (duty_cycle < 10) ? 10 : duty_cycle;
    duty_cycle = (duty_cycle > 990) ? 990 : duty_cycle;

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

void TC1_Handler(void)
{
    uint32_t PIOC_value;
    int8_t left_enc_counts;
    int8_t right_enc_counts;
    int8_t rear_enc_counts;
    
    if ((tc_get_status(TC0, 1) & TC_SR_CPCS) == TC_SR_CPCS)
    {
        /* read encoder ticks */
        pwm_channel_disable(PWM, ENC_CLK);
        PIOC_value = ioport_get_port_level(IOPORT_PIOC, 0xFFFFFFFF);
        ioport_set_pin_level(ENC_LOAD, 0);
        ioport_set_pin_level(ENC_LOAD, 1);
        pwm_channel_enable(PWM, ENC_CLK);
        
        /* extract ticks of each encoder */
        left_enc_counts = (PIOC_value & 0x7F000000) >> 24;
        left_enc_counts = (left_enc_counts & 0x00000040) ? left_enc_counts - 128 : left_enc_counts;
        right_enc_counts = ((PIOC_value & 0x00C00000) >> 17) | ((PIOC_value & 0x001F0000) >> 16);
        right_enc_counts = (right_enc_counts & 0x00000040) ? right_enc_counts - 128 : right_enc_counts;
        rear_enc_counts = ((PIOC_value & 0x0000FC00) >> 9) | ((PIOC_value & 0x00000002) >> 1);
        rear_enc_counts = (rear_enc_counts & 0x00000040) ? rear_enc_counts - 128 : rear_enc_counts;
        
        /* convert ticks to cm/s */
        s.motor.left_speed = (float)left_enc_counts * (CM_PER_TICK / ENCODER_UPDATE_RATE);
        s.motor.right_speed = (float)right_enc_counts * (CM_PER_TICK / ENCODER_UPDATE_RATE);
        s.motor.rear_speed = (float)rear_enc_counts * (CM_PER_TICK / ENCODER_UPDATE_RATE);
    }
}
