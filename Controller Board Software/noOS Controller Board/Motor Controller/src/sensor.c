/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 17.08.2019                                                  */
/************************************************************************/

#include "sensor.h"
#include "timing.h"
#include "string.h"
#include "comm.h"

sensors_t s;

void process_new_sensor_values(void)
{
    static uint32_t last_pi_update = 0;
    
    static uint32_t ticks_line_seen = 0;
    static uint32_t ticks_ball_seen = 0;
    static uint32_t ticks_ball_have = 0;
    static uint32_t ticks_ball_have_2 = 0;
    static uint32_t ticks_goal_seen = 0;
    static uint16_t prev_esc_dir = 0;
    static uint8_t prev_diff = 0;
    static int8_t prev_s_ball_dir = 0;
    static int8_t prev_s_goal_dir = 0;
    static uint8_t prev_s_goal_diff = 0;
    /*static float ball_see_avg = 0.0f;
    static float ball_have_avg = 0.0f;
    static float ball_have_2_avg = 0.0f;
    static float goal_see_avg = 0.0f;
    float ball_see_tmp = 0.0f;
    float ball_have_tmp = 0.0f;
    float ball_have_2_tmp = 0.0f;
    float goal_see_tmp = 0.0f;*/
    
    if((getTicks() - last_pi_update) > 100)
    {
        s.rpi_inactive = true;
        ioport_set_pin_level(LED_M3, 1);
    }
    else
    {
        s.rpi_inactive = false;
        ioport_set_pin_level(LED_M3, 0);
    }
    
    if(new_sc_data_arrived)
    {
        new_sc_data_arrived = false;
        memcpy(&s.line.all, &stm.line.all, sizeof(s.line.all));
        
        if(stm.line.see)
        {
            ticks_line_seen = getTicks();
            s.line.see = 1;
            s.line.esc = stm.line.esc;
            while(s.line.esc > 180) s.line.esc -= 360;
            s.line.diff = stm.line.diff;
            prev_esc_dir = s.line.esc;
            prev_diff = s.line.diff;
        }
        else
        {
            if((getTicks() - ticks_line_seen) > 100)
            {
                s.line.see = 0;
                s.line.esc = 0;
                s.line.diff = 0;
            }
            else
            {
                s.line.see = 1;
                s.line.esc = prev_esc_dir;
                s.line.diff = prev_diff;
            }
        }
        
        memcpy(&s.distance, &stm.distance, sizeof(s.distance));
        memcpy(&s.battery, &stm.battery, sizeof(s.battery));
    }

    if (new_pi_data_arrived)
    {
        new_pi_data_arrived = false;
        last_pi_update = getTicks();
        
        update_pid_goal = true;

        /*ball_see_tmp = (rtm.ball.see) ? 0.1f : 0.0f;
        ball_have_tmp = (rtm.ball.have) ? 0.1f : 0.0f;
        ball_have_2_tmp = (rtm.ball.have_2) ? 0.1f : 0.0f;
        goal_see_tmp = (rtm.goal.see) ? 0.1f : 0.0f;

        ball_see_avg = ball_see_avg * 0.9 + ball_see_tmp;
        ball_have_avg = ball_have_avg * 0.9 + ball_have_tmp;
        ball_have_2_avg = ball_have_2_avg * 0.9 + ball_have_2_tmp;
        goal_see_avg = goal_see_avg * 0.9 + goal_see_tmp;

        s.ball.see = (ball_see_avg > 0.1) ? true : false;
        s.ball.have = (ball_have_avg > 0.1) ? true : false;
        s.ball.have_2 = (ball_have_2_avg > 0.1) ? true : false;
        s.goal.see = (goal_see_avg > 0.1) ? true : false;*/
        
        if(rtm.ball.see)
        {
            ticks_ball_seen = getTicks();
            s.ball.see = true;
            s.ball.dir = (rtm.ball.dir - 32) * 2;
            prev_s_ball_dir = s.ball.dir;
        }
        else
        {
            if((getTicks() - ticks_ball_seen) > 100)
            {
                s.ball.see = false;
                s.ball.dir = 0;
            }
            else
            {
                s.ball.see = true;
                s.ball.dir = prev_s_ball_dir;
            }
        }
        
        if(rtm.ball.have)
        {
            ticks_ball_have = getTicks();
            s.ball.have = true;
        }
        else
        {
            if((getTicks() - ticks_ball_have) > 100)
            {
                s.ball.have = false;
            }
            else
            {
                s.ball.have = true;
            }
        }
        
        if(rtm.ball.have_2)
        {
            ticks_ball_have_2 = getTicks();
            s.ball.have_2 = true;
        }
        else
        {
            if((getTicks() - ticks_ball_have_2) > 100)
            {
                s.ball.have_2 = false;
            }
            else
            {
                s.ball.have_2 = true;
            }
        }
        
        if(rtm.goal.see)
        {
            ticks_goal_seen = getTicks();
            s.goal.see = true;
            s.goal.dir = (rtm.goal.dir - 32) * 2;
            s.goal.diff = rtm.goal.diff * 2;
            prev_s_goal_dir = s.goal.dir;
            prev_s_goal_diff = s.goal.diff;
        }
        else
        {
            if((getTicks() - ticks_goal_seen) > 100)
            {
                s.goal.see = false;
                s.goal.dir = 0;
                s.goal.diff = 0;
            }
            else
            {
                s.goal.see = true;
                s.goal.dir = prev_s_goal_dir;
                s.goal.diff = prev_s_goal_diff;
            }
        }

        /*if(rtm.ball.see)
        {
            s.ball.dir = (rtm.ball.dir - 32) * 2;
            prev_s_ball_dir = s.ball.dir;
        }
        else
        {
            s.ball.dir = prev_s_ball_dir;
        }

        if(rtm.goal.see)
        {
            s.goal.dir = (rtm.goal.dir - 32) * 2;
            s.goal.diff = rtm.goal.diff * 2;
            prev_s_goal_dir = s.goal.dir;
            prev_s_goal_diff = s.goal.diff;
        }
        else
        {
            s.goal.dir = prev_s_goal_dir;
            s.goal.diff = prev_s_goal_diff;
        }*/
    }
}
