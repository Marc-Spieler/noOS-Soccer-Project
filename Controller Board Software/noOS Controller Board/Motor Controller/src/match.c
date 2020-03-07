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
#include "compass.h"
#include "pid.h"

#define TURN_NONE       0
#define TURN_COMPASS    1
#define TURN_GOAL       2

pidReg_t pid_compass;
float pid_compass_out = 0.0f;
uint8_t turn_target = TURN_NONE;

Bool first_loop = false;

Bool arrived_rear = false;
float robot_speed = 0.0f;
float robot_dir = 0.0f;
float robot_trn = 0.0f;

static void move2ball(void);
static void ball2goal(void);
static void return2goal(Bool pos);

void match(void)
{
	if(first_loop)
	{
		first_loop = false;
		pid_compass.kp = 0.7f;
		pid_compass.ki = 0.0f;
		pid_compass.kc = 0.0f;
		pid_compass.kd = 0.6f;
		pid_compass.outMin = -150.0f;
		pid_compass.outMax = 150.0f;
		pid_compass.intg = 0.0f;
		pid_compass.prevErr = 0.0f;
		pid_compass.satErr = 0.0f;
	}
	
	robot_dir = 0.0f;
	robot_speed = 0.0f;
	robot_trn = 0.0f;
	
    estimate_rel_deviation();
    
    ioport_set_pin_level(LED_M1, 0);
    ioport_set_pin_level(LED_M2, 0);
    
    if(update_pid_compass)
    {
        update_pid_compass = false;
        pid_compass_out = pidReg_compass(&pid_compass, 0, -s.compass);
    }
    
    /*if(bt_rx.sbyte.active)
    {
		#if 0
		bt_tx.sbyte.at_goal = false;
		
		if(bt_rx.sbyte.ball_see && s.ball.see)
		{
			if(abs(bt_rx.ball_angle) > abs(s.ball.dir))
			{
				if(s.ball.have || s.ball.have_2)
				{
					ball2goal();
				}
				else
				{
					move2ball();
				}
			}
			else
			{
				bt_rx.sbyte.at_goal = true;
				return2goal(true);
			}
		}
		else if(!bt_rx.sbyte.ball_see && s.ball.see)
		{
			if(s.ball.have || s.ball.have_2)
			{
				ball2goal();
			}
			else
			{
				move2ball();
			}
		}
		else
		{
			if(bt_rx.sbyte.at_goal)
			{
				return2goal(false);
			}
			else
			{
				bt_tx.sbyte.at_goal = true;
				return2goal(true);
			}
		}
		#endif
    }
    else
    {*/
        if(!s.ball.see && ((s.distance.one.arrived && robot_id == 1) || (s.distance.two.arrived && robot_id == 2)))
        {
            arrived_rear = true;
        }
        else
        {
            arrived_rear = false;
        }
        
        turn_target = TURN_COMPASS;

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
				return2goal(robot_id == 2 ? true : false);
            }
        }
    //}
    
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
    
    if(turn_target == TURN_COMPASS)
    {
        robot_trn = pid_compass_out;
    }
    else if(turn_target == TURN_GOAL)
    {
		robot_trn = -s.goal.dir;
    }
    else
    {
        robot_trn = 0.0f;
    }
    
    set_motor(robot_speed, robot_dir, robot_trn);
}

static void move2ball(void)
{
	if(abs(s.ball.dir) < 64)
	{
		if(s.ball.dir <= 20)
		{
			robot_dir = (float)s.ball.dir;
		}
		else
		{
			robot_dir = (float)s.ball.dir * 2.2f;
		}
	}
    else
	{
		if(s.ball.dir < 0)
		{
			robot_dir = 135.0f;
		}
		else
		{
			robot_dir = -135.0f;
		}
	}
    robot_speed = 85.0f;
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
            if(s.ball.have)
            {
	            kick_ball();
            }
        }
        
        turn_target = TURN_GOAL;
    }
}

static void return2goal(Bool pos)
{
	if(!arrived_rear)
	{
		robot_speed = 75.0f;
			
		if((!pos && !s.distance.one.arrived && !s.distance.one.correction_dir) || \
			(pos && !s.distance.two.arrived && !s.distance.two.correction_dir))
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
	if(kicker_percentage >= 70 && (getTicks() - ticks_kicked) >= 100)
	{
		ticks_kicked = getTicks();
		ioport_set_pin_level(KICK_TRIG, 1);
	}
}
