/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 30.11.18                                                    */
/************************************************************************/

#include "distance.h"

pwm_channel_t front_dist_channel;
pwm_channel_t left_dist_channel;
pwm_channel_t right_dist_channel;
pwm_channel_t rear_dist_channel;

#define TICKS_TO_CM     (128.0 / 84000000.0 * 17.15 * 1000)   // CLK-Setting * 34.3cm/ms

void init_distance(void)
{
    /* Initialize PWM channel for DFront */
    /* Period is left-aligned */
    front_dist_channel.alignment = PWM_ALIGN_LEFT;
    /* Output waveform starts at a low level */
    front_dist_channel.polarity = PWM_LOW;
    /* Use PWM clock A as source clock */
    front_dist_channel.ul_prescaler = PWM_CMR_CPRE_CLKA;
    /* Period value of output waveform */
    front_dist_channel.ul_period = PERIOD_VALUE;
    /* Duty cycle value of output waveform */
    front_dist_channel.ul_duty = INIT_DUTY_VALUE;
    front_dist_channel.channel = DFRONT_TRIG;
    pwm_channel_init(PWM, &front_dist_channel);
    
    
    /* Initialize PWM channel for DLeft */
    /* Period is left-aligned */
    left_dist_channel.alignment = PWM_ALIGN_LEFT;
    /* Output waveform starts at a low level */
    left_dist_channel.polarity = PWM_LOW;
    /* Use PWM clock A as source clock */
    left_dist_channel.ul_prescaler = PWM_CMR_CPRE_CLKA;
    /* Period value of output waveform */
    left_dist_channel.ul_period = PERIOD_VALUE;
    /* Duty cycle value of output waveform */
    left_dist_channel.ul_duty = INIT_DUTY_VALUE;
    left_dist_channel.channel = DLEFT_TRIG;
    pwm_channel_init(PWM, &left_dist_channel);
    
    
    /* Initialize PWM channel for DRight */
    /* Period is left-aligned */
    right_dist_channel.alignment = PWM_ALIGN_LEFT;
    /* Output waveform starts at a low level */
    right_dist_channel.polarity = PWM_LOW;
    /* Use PWM clock A as source clock */
    right_dist_channel.ul_prescaler = PWM_CMR_CPRE_CLKA;
    /* Period value of output waveform */
    right_dist_channel.ul_period = PERIOD_VALUE;
    /* Duty cycle value of output waveform */
    right_dist_channel.ul_duty = INIT_DUTY_VALUE;
    right_dist_channel.channel = DRIGHT_TRIG;
    pwm_channel_init(PWM, &right_dist_channel);
    
    
    /* Initialize PWM channel for DBack */
    /* Period is left-aligned */
    rear_dist_channel.alignment = PWM_ALIGN_LEFT;
    /* Output waveform starts at a low level */
    rear_dist_channel.polarity = PWM_LOW;
    /* Use PWM clock A as source clock */
    rear_dist_channel.ul_prescaler = PWM_CMR_CPRE_CLKA;
    /* Period value of output waveform */
    rear_dist_channel.ul_period = PERIOD_VALUE;
    /* Duty cycle value of output waveform */
    rear_dist_channel.ul_duty = INIT_DUTY_VALUE;
    rear_dist_channel.channel = DREAR_TRIG;
    pwm_channel_init(PWM, &rear_dist_channel);

    pwm_channel_enable(PWM, DFRONT_TRIG);
    pwm_channel_enable(PWM, DLEFT_TRIG);
    pwm_channel_enable(PWM, DRIGHT_TRIG);
    pwm_channel_enable(PWM, DREAR_TRIG);
    
    sysclk_enable_peripheral_clock(ID_TC1);
    sysclk_enable_peripheral_clock(ID_TC2);
    sysclk_enable_peripheral_clock(ID_TC6);
    sysclk_enable_peripheral_clock(ID_TC7);
    tc_init(TC0, 1, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_LDRA_RISING | TC_CMR_LDRB_FALLING | TC_CMR_ABETRG | TC_CMR_ETRGEDG_FALLING);
    tc_init(TC0, 2, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_LDRA_RISING | TC_CMR_LDRB_FALLING | TC_CMR_ABETRG | TC_CMR_ETRGEDG_FALLING);
    tc_init(TC2, 0, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_LDRA_RISING | TC_CMR_LDRB_FALLING | TC_CMR_ABETRG | TC_CMR_ETRGEDG_FALLING);
    tc_init(TC2, 1, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_LDRA_RISING | TC_CMR_LDRB_FALLING | TC_CMR_ABETRG | TC_CMR_ETRGEDG_FALLING);

    NVIC_DisableIRQ(TC1_IRQn);
    NVIC_ClearPendingIRQ(TC1_IRQn);
    NVIC_SetPriority(TC1_IRQn, 0);
    NVIC_EnableIRQ(TC1_IRQn);

    NVIC_DisableIRQ(TC2_IRQn);
    NVIC_ClearPendingIRQ(TC2_IRQn);
    NVIC_SetPriority(TC2_IRQn, 0);
    NVIC_EnableIRQ(TC2_IRQn);

    NVIC_DisableIRQ(TC6_IRQn);
    NVIC_ClearPendingIRQ(TC6_IRQn);
    NVIC_SetPriority(TC6_IRQn, 0);
    NVIC_EnableIRQ(TC6_IRQn);

    NVIC_DisableIRQ(TC7_IRQn);
    NVIC_ClearPendingIRQ(TC7_IRQn);
    NVIC_SetPriority(TC7_IRQn, 0);
    NVIC_EnableIRQ(TC7_IRQn);

    tc_enable_interrupt(TC0, 1, TC_IER_LDRBS);
    tc_enable_interrupt(TC0, 2, TC_IER_LDRBS);
    tc_enable_interrupt(TC2, 0, TC_IER_LDRBS);
    tc_enable_interrupt(TC2, 1, TC_IER_LDRBS);
    tc_start(TC0, 1);
    tc_start(TC0, 2);
    tc_start(TC2, 0);
    tc_start(TC2, 1);
}

void update_distance(void)
{
    double front;
    double left;
    double right;
    double rear;
    
    static double prev_front = 0.0;
    static double prev_left = 0.0;
    static double prev_right = 0.0;
    static double prev_rear = 0.0;
    
    double diff_front = 0.0;
    double diff_left = 0.0;
    double diff_right = 0.0;
    double diff_rear = 0.0;
    
    tc_disable_interrupt(TC0, 1, TC_IER_LDRBS);
    front = (double)dfront * TICKS_TO_CM;
    tc_enable_interrupt(TC0, 1, TC_IER_LDRBS);
    tc_disable_interrupt(TC2, 0, TC_IER_LDRBS);
    left = (double)dleft * TICKS_TO_CM;
    tc_enable_interrupt(TC2, 0, TC_IER_LDRBS);
    tc_disable_interrupt(TC2, 1, TC_IER_LDRBS);
    right = (double)dright * TICKS_TO_CM;
    tc_enable_interrupt(TC2, 1, TC_IER_LDRBS);
    tc_disable_interrupt(TC0, 2, TC_IER_LDRBS);
    rear = (double)drear * TICKS_TO_CM;
    tc_enable_interrupt(TC0, 2, TC_IER_LDRBS);

    if (front > 254.0)
    {
        if (front > 2000.0)
        {
            front = 255.0;
        }
        else
        {
            front = 254.0;
        }
    }
    else if (front < 0.0)
    {
        front = 255.0;
    }
    else
    {
        front = prev_front * 0.9 + front * 0.1;
    }
    
    sensor_values.front = (uint8_t)front;
    
    if (left > 254.0)
    {
        if (left > 2000.0)
        {
            left = 255.0;
        }
        else
        {
            left = 254.0;
        }
    }
    else if (left < 0.0)
    {
        left = 255.0;
    }
    else
    {
        left = prev_left * 0.9 + left * 0.1;
    }
    
    sensor_values.left = (uint8_t)left;
    
    if (right > 254.0)
    {
        if (right > 2000.0)
        {
            right = 255.0;
        }
        else
        {
            right = 254.0;
        }
    }
    else if (right < 0.0)
    {
        right = 255.0;
    }
    else
    {
        right = prev_right * 0.9 + right * 0.1;
    }
    
    sensor_values.right = (uint8_t)right;
    
    if (rear > 254.0)
    {
        if (rear > 2000.0)
        {
            rear = 255.0;
        }
        else
        {
            rear = 254.0;
        }
    }
    else if (rear < 0.0)
    {
        rear = 255.0;
    }
    else
    {
        rear = prev_rear * 0.9 + rear * 0.1;
    }
    
    sensor_values.rear = (uint8_t)rear;
    
    if (front < 255.0)
    {
        diff_front = fabs(front - prev_front);
    }
    else
    {
        diff_front = 255.0;
    }
    
    if (left < 255.0)
    {
        diff_left = fabs(left - prev_left);
    }
    else
    {
        diff_left = 255.0;
    }
    
    if (right < 255.0)
    {
        diff_right = fabs(right - prev_right);
    }
    else
    {
        diff_right = 255.0;
    }
    
    if (rear < 255.0)
    {
        diff_rear = fabs(rear - prev_rear);
    }
    else
    {
        diff_rear = 255.0;
    }
    
    if (diff_front < diff_rear)
    {
        sensor_values.rbt_pos.y = (uint8_t)(15.0 - (front / segment_size_Y));
    }
    else
    {
        sensor_values.rbt_pos.y = (uint8_t)(rear / segment_size_Y);
    }
    
    if (diff_left < diff_right)
    {
        sensor_values.rbt_pos.x = (uint8_t)(left / segment_size_X);
    }
    else
    {
        sensor_values.rbt_pos.x = (uint8_t)(15.0 - (right / segment_size_X));
    }
    
    prev_front = front;
    prev_left = left;
    prev_right = right;
    prev_rear = rear;
}

void TC1_Handler(void)
{
    if ((tc_get_status(TC0, 1) & TC_SR_LDRBS) == TC_SR_LDRBS)
    {
        dfront = tc_read_rb(TC0, 1) - tc_read_ra(TC0, 1);
    }
}

void TC2_Handler(void)
{
    if ((tc_get_status(TC0, 2) & TC_SR_LDRBS) == TC_SR_LDRBS)
    {
        drear = tc_read_rb(TC0, 2) - tc_read_ra(TC0, 2);
    }
}

void TC6_Handler(void)
{
    if ((tc_get_status(TC2, 0) & TC_SR_LDRBS) == TC_SR_LDRBS)
    {
        dleft = tc_read_rb(TC2, 0) - tc_read_ra(TC2, 0);
    }
}

void TC7_Handler(void)
{
    if ((tc_get_status(TC2, 1) & TC_SR_LDRBS) == TC_SR_LDRBS)
    {
        dright = tc_read_rb(TC2, 1) - tc_read_ra(TC2, 1);
    }
}
