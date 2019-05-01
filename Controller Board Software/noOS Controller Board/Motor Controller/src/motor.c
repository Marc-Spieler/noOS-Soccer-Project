/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.03.2019                                                  */
/************************************************************************/

#include "motor.h"
#include "pid.h"
#include "asf.h"
#include "string.h"

pwm_channel_t g_pwm_channel_MLeft;
pwm_channel_t g_pwm_channel_MRight;
pwm_channel_t g_pwm_channel_MRear;
pwm_channel_t g_pwm_channel_ENC;    // do not move - weird thing happen

pidReg_t mleft_pid_reg;
pidReg_t mright_pid_reg;
pidReg_t mrear_pid_reg;

float speed_mleft;
float speed_mright;
float speed_mrear;

int16_t mleft_out;
int16_t mright_out;
int16_t mrear_out;

int16_t opponent_goal;
int16_t own_goal;
//int16_t rel_deviation;
float mleft;
float mright;
float mrear;
float SinMA1 = 0.5f;
float SinMA2 = 0.5f;
float SinMA3 = -1.0f;
float CosinMA1 = -0.866025404f;
float CosinMA2 = 0.866025404f;
float CosinMA3 = 0.0f;

uint16_t log_cnt = 0;
int8_t eleft_counts_log[400]; //1200
int8_t eright_counts_log[400];
int8_t erear_counts_log[400];
int8_t speed_mleft_log[400];
int8_t speed_mright_log[400];
int8_t speed_mrear_log[400];
int16_t mleft_out_log[400];
int16_t mright_out_log[400];
int16_t mrear_out_log[400];

void motor_init(void)
{
    /* Initialize PWM channel for MLeft */
    g_pwm_channel_MLeft.alignment = PWM_ALIGN_LEFT;
    g_pwm_channel_MLeft.polarity = PWM_LOW;
    g_pwm_channel_MLeft.ul_prescaler = PWM_CMR_CPRE_CLKA;
    g_pwm_channel_MLeft.ul_period = PERIOD_VALUE;
    g_pwm_channel_MLeft.ul_duty = INIT_DUTY_VALUE;
    g_pwm_channel_MLeft.channel = MOTOR_LEFT;
    pwm_channel_init(PWM, &g_pwm_channel_MLeft);


    /* Initialize PWM channel for MRight */
    g_pwm_channel_MRight.alignment = PWM_ALIGN_LEFT;
    g_pwm_channel_MRight.polarity = PWM_LOW;
    g_pwm_channel_MRight.ul_prescaler = PWM_CMR_CPRE_CLKA;
    g_pwm_channel_MRight.ul_period = PERIOD_VALUE;
    g_pwm_channel_MRight.ul_duty = INIT_DUTY_VALUE;
    g_pwm_channel_MRight.channel = MOTOR_RIGHT;
    pwm_channel_init(PWM, &g_pwm_channel_MRight);


    /* Initialize PWM channel for MBack */
    g_pwm_channel_MRear.alignment = PWM_ALIGN_LEFT;
    g_pwm_channel_MRear.polarity = PWM_LOW;
    g_pwm_channel_MRear.ul_prescaler = PWM_CMR_CPRE_CLKA;
    g_pwm_channel_MRear.ul_period = PERIOD_VALUE;
    g_pwm_channel_MRear.ul_duty = INIT_DUTY_VALUE;
    g_pwm_channel_MRear.channel = MOTOR_REAR;
    pwm_channel_init(PWM, &g_pwm_channel_MRear);

    pwm_channel_disable(PWM, MOTOR_LEFT);
    pwm_channel_disable(PWM, MOTOR_RIGHT);
    pwm_channel_disable(PWM, MOTOR_REAR);

    /* Initialize PWM channel for Encoders */
    g_pwm_channel_ENC.alignment = PWM_ALIGN_LEFT;
    g_pwm_channel_ENC.polarity = PWM_LOW;
    g_pwm_channel_ENC.ul_prescaler = PWM_CMR_CPRE_CLKA;
    g_pwm_channel_ENC.ul_period = 330;
    g_pwm_channel_ENC.ul_duty = 165;
    g_pwm_channel_ENC.channel = ENC_CLK;
    pwm_channel_init(PWM, &g_pwm_channel_ENC);
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

    mleft_pid_reg.kp = 10.0f;
    mleft_pid_reg.ki = 0.7f;
    mleft_pid_reg.kc = 1.0f;
    mleft_pid_reg.kd = 0.0f;
    mleft_pid_reg.outMin = -500.0f;
    mleft_pid_reg.outMax = 500.0f;

    mright_pid_reg = mleft_pid_reg;
    mrear_pid_reg = mleft_pid_reg;
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
}

void update_motor(float mleft_ref, float mright_ref, float mrear_ref)
{
    tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
    speed_mleft = (float)mleft_ref;
    speed_mright = (float)mright_ref;
    speed_mrear = (float)mrear_ref;
    tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
}

void motor_speed(uint8_t motor, int16_t ispeed)
{
    if (ispeed > 500)
    {
        ispeed = 500;
    }

    if (ispeed < -500)
    {
        ispeed = -500;
    }

    uint16_t duty_cycle = (uint16_t)(ispeed + (int16_t)INIT_DUTY_VALUE);

    if (duty_cycle < 10)
    {
        duty_cycle = 10;
    }

    if (duty_cycle > 990)
    {
        duty_cycle = 990;
    }

    switch(motor)
    {
        case MOTOR_LEFT:
            pwm_channel_update_duty(PWM, &g_pwm_channel_MLeft, duty_cycle);
            break;
        case MOTOR_RIGHT:
            pwm_channel_update_duty(PWM, &g_pwm_channel_MRight, duty_cycle);
            break;
        case MOTOR_REAR:
            pwm_channel_update_duty(PWM, &g_pwm_channel_MRear, duty_cycle);
            break;
        default:
            break;
    }
}

void TC1_Handler(void)
{
    uint32_t PIOC_value;
    int8_t eleft_counts;
    int8_t eright_counts;
    int8_t erear_counts;

    ioport_set_pin_level(LED_M3, 1);

    if ((tc_get_status(TC0, 1) & TC_SR_CPCS) == TC_SR_CPCS)
    {
        pwm_channel_disable(PWM, ENC_CLK);
        PIOC_value = ioport_get_port_level(IOPORT_PIOC, 0xFFFFFFFF);
        ioport_set_pin_level(ENC_LOAD, 0);
        ioport_set_pin_level(ENC_LOAD, 1);
        pwm_channel_enable(PWM, ENC_CLK);

        eleft_counts = (PIOC_value & 0x7F000000) >> 24;
        eleft_counts = (eleft_counts & 0x00000040) ? eleft_counts - 128 : eleft_counts;
        eright_counts = ((PIOC_value & 0x00C00000) >> 17) | ((PIOC_value & 0x001F0000) >> 16);
        eright_counts = (eright_counts & 0x00000040) ? eright_counts - 128 : eright_counts;
        erear_counts = ((PIOC_value & 0x0000FC00) >> 9) | ((PIOC_value & 0x00000002) >> 1);
        erear_counts = (erear_counts & 0x00000040) ? erear_counts - 128 : erear_counts;

        mleft_out = pidReg(&mleft_pid_reg, speed_mleft, (float)eleft_counts);
        mright_out = pidReg(&mright_pid_reg, speed_mright, (float)eright_counts);
        mrear_out = pidReg(&mrear_pid_reg, speed_mrear, (float)erear_counts);
        motor_speed(MOTOR_LEFT, mleft_out);
        motor_speed(MOTOR_RIGHT, mright_out);
        motor_speed(MOTOR_REAR, mrear_out);

        /*if(log_cnt < 400)
        {
            eleft_counts_log[log_cnt] = eleft_counts;
            eright_counts_log[log_cnt] = eright_counts;
            erear_counts_log[log_cnt] = erear_counts;
            speed_mleft_log[log_cnt] = (int8_t)speed_mleft;
            speed_mright_log[log_cnt] = (int8_t)speed_mright;
            speed_mrear_log[log_cnt] = (int8_t)speed_mrear;
            mleft_out_log[log_cnt] = mleft_out;
            mright_out_log[log_cnt] = mright_out;
            mrear_out_log[log_cnt] = mrear_out;
            log_cnt++;
        }
        else
        {
            tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
            disable_motor();
            UINT bw;
            FIL file_object;
            char test_file_name[] = "motor_logging_p10_i0.7_c1_d0_+-5_teppich.txt";
            char sprintf_buf[11];
            f_open(&file_object, (char const *)test_file_name, FA_CREATE_ALWAYS | FA_WRITE);
            
            for(int i = 0; i < log_cnt; i++)
            {
                sprintf(sprintf_buf, "%5d;", i);
                f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);

                sprintf(sprintf_buf, "%3d;", eleft_counts_log[i]);
                f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
                sprintf(sprintf_buf, "%3d;", eright_counts_log[i]);
                f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
                sprintf(sprintf_buf, "%3d;", erear_counts_log[i]);
                f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);

                sprintf(sprintf_buf, "%3d;", speed_mleft_log[i]);
                f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
                sprintf(sprintf_buf, "%3d;", speed_mright_log[i]);
                f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
                sprintf(sprintf_buf, "%3d;", speed_mrear_log[i]);
                f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);

                sprintf(sprintf_buf, "%4d;", mleft_out_log[i]);
                f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
                sprintf(sprintf_buf, "%4d;", mright_out_log[i]);
                f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
                sprintf(sprintf_buf, "%4d\r\n", mrear_out_log[i]);
                f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
            }

            f_close(&file_object);
            ioport_set_pin_level(LED_ONBOARD, 1);
            while(1);
        }*/
        //pwm_channel_enable(PWM, ENC_CLK);
    }

    ioport_set_pin_level(LED_M3, 0);
}

/*void set_opponent_goal(void)
{
    update_compass();
    opponent_goal = direction;
}
void set_inv_opponent_goal(void)
{
    update_compass();
    opponent_goal = direction + 180;;
}
void set_own_goal(void)
{
    update_compass();
    own_goal = direction;
}
int16_t estimate_rel_deviation(uint16_t dir, int16_t tar)
{
    int16_t rel_dev = dir - tar;
    if (rel_dev >= 1800)
    {
        rel_dev -= 3600;
    }
    
    if (rel_dev <= -1800)
    {
        rel_dev += 3600;
    }
    
    return rel_dev;
}
*/
