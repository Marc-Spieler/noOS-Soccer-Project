/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 16.06.2019                                                  */
/************************************************************************/

#include "distance.h"
#include "comm.h"
#include "math.h"

#define TICKS_TO_CM     (128.0 / 84000000.0 * 17.15 * 1000)   // CLK-Setting * 34.3cm/ms

pwm_channel_t distance_pwm;

static uint32_t time_of_flight;

void distance_init(void)
{
    /* Initialize distance PWM channel */
    distance_pwm.alignment = PWM_ALIGN_LEFT;
    distance_pwm.polarity = PWM_LOW;
    distance_pwm.ul_prescaler = PWM_CMR_CPRE_CLKA;
    distance_pwm.ul_period = PERIOD_VALUE;
    distance_pwm.ul_duty = INIT_DUTY_VALUE;
    distance_pwm.channel = DISTANCE_TRIG;
    pwm_channel_init(PWM, &distance_pwm);
    pwm_channel_enable(PWM, DISTANCE_TRIG);

    sysclk_enable_peripheral_clock(ID_TC1);
    tc_init(TC0, 1, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_LDRA_RISING | TC_CMR_LDRB_FALLING | TC_CMR_ABETRG | TC_CMR_ETRGEDG_FALLING);

    NVIC_DisableIRQ(TC1_IRQn);
    NVIC_ClearPendingIRQ(TC1_IRQn);
    NVIC_SetPriority(TC1_IRQn, 0);
    NVIC_EnableIRQ(TC1_IRQn);

    tc_enable_interrupt(TC0, 1, TC_IER_LDRBS);
    tc_start(TC0, 1);
}

void update_distance(void)
{
    static double distance;
    static uint32_t prev_tof;

    if(time_of_flight != prev_tof)
    {
        tc_disable_interrupt(TC0, 1, TC_IER_LDRBS);
        distance = (double)time_of_flight * TICKS_TO_CM;
        tc_enable_interrupt(TC0, 1, TC_IER_LDRBS);
        
        if(distance >= 20 && distance <= 25)
        {
            stm.distance_1 = true;
            ioport_set_pin_level(LED_S1, 1);
        }
        else
        {
            stm.distance_1 = false;
            ioport_set_pin_level(LED_S1, 0);
        }

        if(distance >= 10 && distance <= 15)
        {
            stm.distance_2 = true;
            ioport_set_pin_level(LED_S2, 1);
        }
        else
        {
            stm.distance_2 = false;
            ioport_set_pin_level(LED_S2, 0);
        }
        
        /*if(distance > 2000.0f || distance < 0.0f)
        {
            stm.distance = 255;
        }
        else if(distance > 254.0f)
        {
            stm.distance = 254;
        }
        else
        {
            stm.distance = (uint8_t)distance;
        }*/

        prev_tof = time_of_flight;
    }
}

void TC1_Handler(void)
{
    if((tc_get_status(TC0, 1) & TC_SR_LDRBS) == TC_SR_LDRBS)
    {
        time_of_flight = tc_read_rb(TC0, 1) - tc_read_ra(TC0, 1);
    }
}
