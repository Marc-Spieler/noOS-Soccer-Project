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
#include "pid.h"

#define CM_PER_TICK             ((2 * M_PI * 3) / 464.64)
#define ENCODER_UPDATE_RATE     (0.008) //in seconds

#define DOUBLE_RES 1

pwm_channel_t pwm_motor_left;
pwm_channel_t pwm_motor_right;
pwm_channel_t pwm_motor_rear;
pwm_channel_t pwm_encoder;    // do not move - weird thing happens

pidReg_t pid_motor_left;
pidReg_t pid_motor_right;
pidReg_t pid_motor_rear;

float motor_left_target;
float motor_right_target;
float motor_rear_target;

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
    tc_write_rc(TC0, 1, 5249);  //MCLK / 128 * ENC_UPDATE_RATE - 1
    NVIC_DisableIRQ(TC1_IRQn);
    NVIC_ClearPendingIRQ(TC1_IRQn);
    NVIC_SetPriority(TC1_IRQn, 0);
    NVIC_EnableIRQ(TC1_IRQn);
    tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
    tc_start(TC0, 1);
    
    pid_motor_left.kp = 30.0f;
    pid_motor_left.ki = 5.0f;
    pid_motor_left.kc = 1.0f;
    pid_motor_left.kd = 10.0f;
    pid_motor_left.outMin = -500.0f;
    pid_motor_left.outMax = 500.0f;

    pid_motor_right = pid_motor_left;
    pid_motor_rear = pid_motor_left;
}

void enable_motor(void)
{
    pid_motor_left.intg = 0.0f;
    pid_motor_left.prevErr = 0.0f;
    pid_motor_left.satErr = 0.0f;
    
    pid_motor_right.intg = 0.0f;
    pid_motor_right.prevErr = 0.0f;
    pid_motor_right.satErr = 0.0f;
    
    pid_motor_rear.intg = 0.0f;
    pid_motor_rear.prevErr = 0.0f;
    pid_motor_rear.satErr = 0.0f;
    
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
    
    set_motor_individual(left, right, rear);
}

void set_motor_individual(int16_t left, int16_t right, int16_t rear)
{
	/* convert ticks to cm/s */
	left /= (CM_PER_TICK / ENCODER_UPDATE_RATE);
	right /= (CM_PER_TICK / ENCODER_UPDATE_RATE);
	rear /= (CM_PER_TICK / ENCODER_UPDATE_RATE);
	
    /* update motor target speed values */
    tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
    motor_left_target = (float)left;
    motor_right_target = (float)right;
    motor_rear_target = (float)rear;
    tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
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
    int8_t enc_counts_left;
    int8_t enc_counts_right;
    int8_t enc_counts_rear;
    
    #if DOUBLE_RES == 1
        const float prev_enc_counts_left = 0.0f;
        const float prev_enc_counts_right = 0.0f;
        const float prev_enc_counts_rear = 0.0f;
    #endif
    
    if ((tc_get_status(TC0, 1) & TC_SR_CPCS) == TC_SR_CPCS)
    {
        /* read encoder ticks */
        pwm_channel_disable(PWM, ENC_CLK);
        PIOC_value = ioport_get_port_level(IOPORT_PIOC, 0xFFFFFFFF);
        ioport_set_pin_level(ENC_LOAD, 0);
        ioport_set_pin_level(ENC_LOAD, 1);
        pwm_channel_enable(PWM, ENC_CLK);
        
        /* extract ticks of each encoder */
        enc_counts_left = (PIOC_value & 0x7F000000) >> 24;
        enc_counts_left = (enc_counts_left & 0x00000040) ? enc_counts_left - 128 : enc_counts_left;
        enc_counts_right = ((PIOC_value & 0x00C00000) >> 17) | ((PIOC_value & 0x001F0000) >> 16);
        enc_counts_right = (enc_counts_right & 0x00000040) ? enc_counts_right - 128 : enc_counts_right;
        enc_counts_rear = ((PIOC_value & 0x0000FC00) >> 9) | ((PIOC_value & 0x00000002) >> 1);
        enc_counts_rear = (enc_counts_rear & 0x00000040) ? enc_counts_rear - 128 : enc_counts_rear;
        
        /* convert ticks to cm/s */
        s.motor.speed.left = (float)enc_counts_left * (CM_PER_TICK / ENCODER_UPDATE_RATE);
        s.motor.speed.right = (float)enc_counts_right * (CM_PER_TICK / ENCODER_UPDATE_RATE);
        s.motor.speed.rear = (float)enc_counts_rear * (CM_PER_TICK / ENCODER_UPDATE_RATE);
        
        #if DOUBLE_RES == 1
            float filtered_enc_counts_left = (enc_counts_left + prev_enc_counts_left) / 2;
            float filtered_enc_counts_right = (enc_counts_right + prev_enc_counts_right) / 2;
            float filtered_enc_counts_rear = (enc_counts_rear + prev_enc_counts_rear) / 2;
        
            /* update individual motor pid controllers */
            s.motor.output.left = pidReg(&pid_motor_left, motor_left_target, filtered_enc_counts_left);
            s.motor.output.right = pidReg(&pid_motor_right, motor_right_target, filtered_enc_counts_right);
            s.motor.output.rear = pidReg(&pid_motor_rear, motor_rear_target, filtered_enc_counts_rear);
        #else
            /* update individual motor pid controllers */
            s.motor.output.left = pidReg(&pid_motor_left, motor_left_target, enc_counts_left);
            s.motor.output.right = pidReg(&pid_motor_right, motor_right_target, enc_counts_right);
            s.motor.output.rear = pidReg(&pid_motor_rear, motor_rear_target, enc_counts_rear);
        #endif
        
        /* write new output values to motors */
        update_motor_pwm(MOTOR_LEFT, s.motor.output.left);
        update_motor_pwm(MOTOR_RIGHT, s.motor.output.right);
        update_motor_pwm(MOTOR_REAR, s.motor.output.rear);
    }
}
