/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 03.03.19                                                    */
/************************************************************************/

#include "match.h"
#include "compass.h"
#include "motor.h"
#include "support.h"
#include "bt.h"
#include "comm.h"
#include "timing.h"

Bool arrived_rear = false;
float robot_speed = 0.0f;
float robot_dir = 0.0f;
float robot_trn = 0.0f;

static void move2ball(void);
static void ball2goal(void);
static void return2goal(void);

void match(void)
{
    estimate_rel_deviation();
    
    ioport_set_pin_level(LED_M1, 0);
    ioport_set_pin_level(LED_M2, 0);
    
    if(bt_rx.sbyte.active)
    {
        /*bt_rx.ball_angle
        bt_rx.ball_dist
        bt_rx.goal_angle
        bt_rx.goal_dist
        
        if()
        {
            
        }*/
    }
    else
    {
        if(!s.ball.see && ((s.distance.one.arrived && robot_id == 1) || (s.distance.two.arrived && robot_id == 2)))
        {
            arrived_rear = true;
        }
        else
        {
            arrived_rear = false;
        }
        
        robot_trn = s.compass;

        if(s.ball.have || s.ball.have_2)
        {
            ball2goal();
        }
        else
        {
            if(s.ball.see)
            {
                move2ball();
            }
            else
            {
                return2goal();
            }
        }
    }
    
    if(s.line.see)
    {
        int16_t esc_min = s.line.esc - s.line.diff;
        int16_t esc_max = s.line.esc + s.line.diff;
        
        while(esc_min <= -180) esc_min += 360;
        while(esc_max > 180) esc_max -= 360;
        
        if(esc_min > esc_max)
        {
            int16_t tmp = esc_min;
            esc_min = esc_max;
            esc_max = tmp;
        }
        
        int16_t esc_dir_diff = s.line.esc - robot_dir;
        
        while(esc_dir_diff <= -180) esc_dir_diff += 360;
        while(esc_dir_diff > 180) esc_dir_diff -= 360;
        
        if(s.line.diff > 0)
        {
            if(abs(esc_dir_diff) <= 90)
            {
                if(robot_dir < esc_min)
                {
                    robot_dir = esc_min;
                    ioport_set_pin_level(LED_M2, 1);
                }
                if(robot_dir > esc_max)
                {
                    robot_dir = esc_max;
                    ioport_set_pin_level(LED_M1, 1);
                }
            }
            else
            {
                robot_dir = robot_dir < 0 ? esc_min : esc_max;
            }
        }
        else
        {
            robot_dir = (float)(s.line.esc);
            robot_speed = 75.0f;
        }
    }
    
    set_motor(robot_speed, robot_dir, robot_trn);
}

static void move2ball(void)
{
    robot_dir = (float)s.ball.dir * 2.2f;
    robot_speed = 75.0f;
}

static void ball2goal(void)
{
    robot_speed = 100.0f;
    
    if(s.goal.see)
    {
        if(abs(s.goal.dir) > s.goal.diff) // if not facing goal directly
        {
            robot_speed = 50.0f;
        }
        else
        {
            // if ball in kick position
            // kick
        }
        
        robot_trn = -s.goal.dir;
    }
}

static void return2goal(void)
{
    if(!arrived_rear)
    {
        robot_speed = 75.0f;
        
        if((robot_id == 1 && !s.distance.one.arrived && !s.distance.one.correction_dir)\
        || (robot_id == 2 && !s.distance.two.arrived && !s.distance.two.correction_dir))
        {
            if(s.goal.see && (s.goal.dir < -10 || s.goal.dir > 10)) // when goal visible center on field using goal as reference
            {
                if(s.goal.dir < -10)
                {
                    robot_dir = -155.0f;
                }
                else
                {
                    robot_dir = 155.0f;
                }
            }
            else
            {
                robot_dir = 180.0f;
            }
        }
    }
    else
    {
        robot_speed = 30.0f;
        
        if(s.goal.dir < -5)
        {
            robot_dir = -90.0f;
        }
        else if(s.goal.dir > 5)
        {
            robot_dir = 90.0f;
        }
        else
        {
            robot_speed = 0.0f;
        }
    }
}

void kick_ball(void)
{
    ioport_set_pin_level(KICK_TRIG, 1);
    ticks_kicked = getTicks();
}
