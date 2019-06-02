/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 19.08.2018                                                  */
/************************************************************************/

#include "menu.h"
#include "lcd.h"
#include "timing.h"
#include "compass.h"
#include "comm.h"
#include "support.h"
#include "iniparser.h"
#include "motor.h"
#include "math.h"

#define FORCE_LIMIT_ON 0
#define FORCE_LIMIT (225)
#define STANDARD_FORCE (500)

#define SPEED_FAST 70
#define SPEED_SLOW 30

uint32_t ticks_test = 0;

menu_t act_menu = MENU_BOOTUP;
Bool print_menu = true;

FIL noOS_ini_file;
char str[8];

Bool blink_level;
uint32_t ticks_blink_update;
uint32_t ticks_dot_update;
uint8_t dots = 3;
Bool update_dots = 1;

Bool ini_update_robot_id = false;
Bool ini_update_speed = false;
Bool ini_update_heartbeat = false;
Bool ini_update_line_cal = false;

typedef struct
{
    uint8_t act_cursor_line;
    uint8_t prev_cursor_line;
    uint8_t min_cursor_line;
    uint8_t max_cursor_line;
} menu_info_t;

struct
{
    menu_info_t main;
    menu_info_t sensors;
    menu_info_t settings;
    menu_info_t encoder;
} menu_info =
  {
      { 2, 2, 2, 4 },
      { 1, 1, 1, 4 },
      { 1, 1, 1, 3 },
      { 1, 1, 1, 3 }
  };

Bool shutdown_confirmed = 0;
Bool inverted_start = false;

char sprintf_buf[21];

static void menu_main(event_t event1);
static void menu_match(event_t event1);
static void menu_match_magdeburg(event_t event1);
static void menu_match_voehringen(event_t event1);
static void menu_match_voehringen_optimized(event_t event1);
static void menu_test(event_t event1);
static void menu_sensors(event_t event1);
static void menu_camera(event_t event1);
static void menu_compass(event_t event1);
static void menu_compass_calibration(event_t event1);
static void menu_line(event_t event1);
static void menu_line_calibration(event_t event1);
static void menu_encoder(event_t event1);
static void menu_settings(event_t event1);
static void menu_bootup(event_t event1);
static void menu_shutdown(event_t event1);
static void print_menu_main(void);
static void print_menu_sensors(void);
static void print_menu_settings(void);
static void print_cursor(menu_info_t *info);

void menu(event_t event1)
{
    switch (act_menu)
    {
        case MENU_MAIN:
            menu_main(event1);
            break;
        case MENU_MATCH:
            menu_match_magdeburg(event1);
            break;
        case MENU_SENSORS:
            menu_sensors(event1);
            break;
        case MENU_SETTINGS:
            menu_settings(event1);
            break;
        case MENU_CAMERA:
            menu_camera(event1);
            break;
        case MENU_COMPASS:
            menu_compass(event1);
            break;
        case MENU_COMPASS_CALIBRATION:
            menu_compass_calibration(event1);
            break;
        case MENU_LINE:
            menu_line(event1);
            break;
        case MENU_LINE_CALIBRATION:
            menu_line_calibration(event1);
            break;
        case MENU_ENCODER:
            menu_encoder(event1);
            break;
        case MENU_BOOTUP:
            menu_bootup(event1);
            break;
        case MENU_SHUTDOWN:
            menu_shutdown(event1);
            break;
        default:
            break;
    }
}

static void menu_main(event_t event1)
{
    if (print_menu)
    {
        print_menu_main();
    }
    
    update_battery(0);
    
    switch (event1)
    {
        case EVENT_BUTTON_UP_P:
            if (menu_info.main.act_cursor_line > menu_info.main.min_cursor_line)
            {
                menu_info.main.act_cursor_line--;
                print_cursor(&menu_info.main);
            }
            break;
        case EVENT_BUTTON_DOWN_P:
            if (menu_info.main.act_cursor_line < menu_info.main.max_cursor_line)
            {
                menu_info.main.act_cursor_line++;
                print_cursor(&menu_info.main);
            }
            break;
        case EVENT_BUTTON_MID_P:
            {
                switch (menu_info.main.act_cursor_line)
                {
                    case 2:
                        act_menu = MENU_MATCH;
                        print_menu = true;
                        inverted_start = false;
                        break;
                    case 3:
                        act_menu = MENU_SENSORS;
                        print_menu = true;
                        break;
                    case 4:
                        act_menu = MENU_SETTINGS;
                        print_menu = true;
                        break;
                    default:
                        break;
                }
            }
            break;
        case EVENT_BUTTON_RIGHT_P:
            switch (menu_info.main.act_cursor_line)
            {
                case 2:
                    act_menu = MENU_MATCH;
                    print_menu = true;
                    inverted_start = true;
                    break;
                default:
                    break;
            }
            break;
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_SHUTDOWN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}
/*
static void menu_test(event_t event1)
{
    float robot_speed = speed_preset;
    float robot_dir = 0;
    float robot_trn = 0;
    
    if(print_menu)
    {
        lcd_clear();
        set_opponent_goal();
        enable_motor();
    }
    
    estimate_rel_deviation();
    
    set_motor(0, 0, compass_dev / 2);
    
    switch (event1)
    {
        case EVENT_BUTTON_RETURN_P:
            disable_motor();
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}
*/
static void menu_match(event_t event1)
{
    /*
    avaiable sensors:
    - ball
    - goal
    - line
    - compass

    line.see == 0:
        speed = normal;
        ball.having == 1:
            goal.see == 1:
                |goal.dir| < goal.diff:
                    speed = max;
                dir = goal.dir;
            goal.see == 0:
                dir = 0;
        ball.having == 0:
            ball.see == 1:
                |goal.dir| < goal.diff:
                    dir = 0;
                    speed = max;
                else:
                    dir = ball.dir * factor;
            ball.see == 0:
                dir = 180;
    line.see == 1:
        dir = line.esc;
        speed = max;
    */

    float robot_speed = 0;//speed_preset;
    float robot_dir = 0;
    float robot_trn = 0;
    int16_t add_left = 0;
    int16_t add_right = 0;
    int16_t add_rear = 0;
    
    ioport_set_pin_level(LED_M1, 0);
    ioport_set_pin_level(LED_M2, 0);
    ioport_set_pin_level(LED_M3, 0);
    
    if(print_menu)
    {
        lcd_set_backlight(LCD_LIGHT_OFF);
        lcd_clear();    // required to turn backlight on/off
        set_opponent_goal();
        enable_motor();
    }
    
    estimate_rel_deviation();

    if(!s.line.see)
    {
        if(s.ball.have)
        {
            ioport_set_pin_level(LED_M3, 1);
            if(s.goal.see)
            {
                if(abs(s.goal.dir) <= rtm.goal.diff)
                {
                    ioport_set_pin_level(LED_M1, 1);
                    robot_speed = SPEED_FAST;
                    robot_dir = 0;
                    robot_trn -= s.goal.dir;
                }
                else
                {
                    ioport_set_pin_level(LED_M2, 1);
                    robot_speed = 0;
                    robot_dir = 0;
                    robot_trn = 0;
                    
                    if(s.goal.dir > 0)
                    {
                        add_left += -24;
                        add_right += 36;
                        add_rear += -48;
                    }
                    else if(s.goal.dir < 0)
                    {
                        add_left += -36;
                        add_right += 24;
                        add_rear += 48;
                    }
                }
            }
            else
            {
                if(abs(s.compass) < 10)
                {
                    robot_speed = SPEED_FAST;
                    robot_dir = 0;
                    robot_trn += s.compass;
                }
                else
                {
                    robot_speed = 0;
                    robot_dir = 0;
                    robot_trn = 0;
                    
                    if(s.compass > 0)
                    {
                        add_left += 4;//-36
                        add_right += 10;//24
                        add_rear += 60;//48
                    }
                    else if(s.compass < 0)
                    {
                        add_left += -10;//-24
                        add_right += -4;//36
                        add_rear += -60;//-48
                    }
                }
            }
        }
        else
        {
            if(s.ball.see)
            {
                if(s.goal.see)
                {
                    if(abs(s.ball.dir) < 15 && abs(s.goal.dir) <= s.goal.diff)
                    {
                        robot_speed = SPEED_FAST;
                        robot_trn -= s.ball.dir; //
                    }
                    else
                    {
                        ioport_set_pin_level(LED_M3, 1);
                        //robot_dir = s.ball.dir * 2.1;
                        if(s.ball.dir > s.goal.dir)
                        {
                            robot_dir = 90;
                        }
                        else
                        {
                            robot_dir = -90;
                        }
                        
                        /*if(abs(s.goal.dir) <= s.goal.diff)
                        {
                            robot_trn -= s.goal.dir;
                        }
                        else
                        {*/
                            robot_trn -= s.ball.dir;
                        //}                        
                        
                        robot_speed = SPEED_SLOW; //
                    }
                }
                else
                {
                    if(abs(s.ball.dir) < 15 && abs(s.compass) < 15)
                    {
                        robot_speed = SPEED_FAST;
                        robot_dir = s.ball.dir;
                        robot_trn += s.compass;
                    }
                    else
                    {
                        if(s.compass > 0)
                        {
                            robot_dir = 90;
                        }
                        else
                        {
                            robot_dir = -90;
                        }
                        robot_trn -= s.ball.dir;
                        robot_speed = (abs(s.compass > 5)) ? SPEED_SLOW : 0; //
                    }
                }
            }
            else
            {
                ioport_set_pin_level(LED_M1, 1);
                /*robot_speed = 50;
                robot_dir = 180;
                robot_trn += s.compass;*/
            }
        }
    }
    else
    {
        robot_dir = s.line.esc;
        robot_speed = 30;
    }

    set_motor_extended(robot_speed, robot_dir, robot_trn, add_left, add_right, add_rear);

    switch (event1)
    {
        case EVENT_BUTTON_RETURN_P:
            disable_motor();
            lcd_set_backlight(LCD_LIGHT_ON);
            lcd_clear();    // required to turn backlight on/off
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_match_magdeburg(event_t event1)
{
    static Bool arrived_rear = false;
    float robot_speed = speed_preset;
    float robot_dir = 0.0f;
    float robot_trn = 0.0f;
    
    if(print_menu)
    {
        lcd_set_backlight(LCD_LIGHT_OFF);
        lcd_clear();    // required to turn backlight on/off
        
        if(inverted_start)
        {
            set_inverted_opponent_goal();
        }
        else
        {
            set_opponent_goal();
        }
        
        enable_motor();
    }
    
    estimate_rel_deviation();
    
    ioport_set_pin_level(LED_M1, 0);
    ioport_set_pin_level(LED_M2, 0);
    ioport_set_pin_level(LED_M3, 0);
    
    if (!s.line.see)
    {
        if (s.ball.have)
        {
            robot_speed = 100.0f;
            robot_dir = 0.0f;
            
            if(s.goal.see)
            {
                if(abs(s.goal.dir) <= s.goal.diff)
                {
                    robot_speed = 100.0f;
                }
                else
                {
                    robot_speed = 50.0f;
                }
                
                robot_trn = -s.goal.dir;// / 0.6;
            }
            else
            {
                robot_trn = s.compass;// / 2.0f;
            }
#if FORCE_LIMIT_ON == 1
            tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
            pid_motor_left.outMax = STANDARD_FORCE;
            pid_motor_left.outMin = -STANDARD_FORCE;
            pid_motor_right.outMax = STANDARD_FORCE;
            pid_motor_right.outMin = -STANDARD_FORCE;
            pid_motor_rear.outMax = STANDARD_FORCE;
            pid_motor_rear.outMin = -STANDARD_FORCE;
            tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
#endif
        }
        else
        {
            if(s.ball.see)
            {
                robot_dir = (float)(s.ball.dir * 2.1);
                robot_speed = 75.0f;
#if FORCE_LIMIT_ON == 1
                tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
                pid_motor_left.outMax = STANDARD_FORCE;
                pid_motor_left.outMin = -STANDARD_FORCE;
                pid_motor_right.outMax = STANDARD_FORCE;
                pid_motor_right.outMin = -STANDARD_FORCE;
                pid_motor_rear.outMax = STANDARD_FORCE;
                pid_motor_rear.outMin = -STANDARD_FORCE;
                tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
#endif
            }
            else
            {
                robot_speed = 20.0f;//75

                if(!arrived_rear)
                {
                    if(s.goal.see)
                    {
                        if(s.goal.dir < -5)
                        {
                            ioport_set_pin_level(LED_M1, 1);
                            robot_dir = -135.0f;
                        }
                        else if(s.goal.dir > 5)
                        {
                            ioport_set_pin_level(LED_M2, 1);
                            robot_dir = 135.0f;
                        }
                        else
                        {
                            ioport_set_pin_level(LED_M3, 1);
                            robot_dir = 180.0f;
                        }
                    }
                    else
                    {
                        robot_dir = 180.0f;
                    }
                }
                else
                {
                    if(s.goal.dir < -5)
                    {
                        ioport_set_pin_level(LED_M1, 1);
                        robot_dir = -90.0f;
                    }
                    else if(s.goal.dir > 5)
                    {
                        ioport_set_pin_level(LED_M2, 1);
                        robot_dir = 90.0f;
                    }
                    else
                    {
                        ioport_set_pin_level(LED_M3, 1);
                        robot_speed = 0.0f;
                    }

                    arrived_rear = false;
                }
                
#if FORCE_LIMIT_ON == 1
                tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
                pid_motor_left.outMax = FORCE_LIMIT;
                pid_motor_left.outMin = -FORCE_LIMIT;
                pid_motor_right.outMax = FORCE_LIMIT;
                pid_motor_right.outMin = -FORCE_LIMIT;
                pid_motor_rear.outMax = FORCE_LIMIT;
                pid_motor_rear.outMin = -FORCE_LIMIT;
                tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
#endif
            }
            
            robot_trn = s.compass;// / 3.0f;
        }
    }
    else
    {
        robot_dir = (float)(s.line.esc);
        robot_speed = 75.0f;

        if(!s.ball.see)
        {
            arrived_rear = true;
        }
#if FORCE_LIMIT_ON == 1
        tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
        pid_motor_left.outMax = STANDARD_FORCE;
        pid_motor_left.outMin = -STANDARD_FORCE;
        pid_motor_right.outMax = STANDARD_FORCE;
        pid_motor_right.outMin = -STANDARD_FORCE;
        pid_motor_rear.outMax = STANDARD_FORCE;
        pid_motor_rear.outMin = -STANDARD_FORCE;
        tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
#endif
    }

    /*robot_dir *= (3.14159265359f / 180.0f);
    
    mleft = robot_speed * (cos(robot_dir) * CosinMA1 - sin(robot_dir) * SinMA1);
    mright = robot_speed * (cos(robot_dir) * CosinMA2 - sin(robot_dir) * SinMA2);
    mrear = robot_speed * (cos(robot_dir) * CosinMA3 - sin(robot_dir) * SinMA3);
    
    mleft += robot_trn;
    mright += robot_trn;
    mrear += robot_trn;
    
    compensate_motor_output();
    
    set_motor_individual(mleft, mright, mrear);*/
    set_motor(robot_speed, robot_dir, robot_trn);

    switch (event1)
    {
        case EVENT_BUTTON_RETURN_P:
            disable_motor();
            lcd_set_backlight(LCD_LIGHT_ON);
            lcd_clear();    // required to turn backlight on/off
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}
#if 0
static void menu_match_voehringen_optimized(event_t event1)
{
    float robot_speed = speed_preset;
    float robot_dir = 0;
    
    if(print_menu)
    {
        lcd_set_backlight(LCD_LIGHT_OFF);
        lcd_clear();    // required to turn backlight on/off
        
        if(inverted_start)
        {
            set_inverted_opponent_goal();
        }
        else
        {
            set_opponent_goal();
        }
        
        enable_motor();
    }
    
    estimate_rel_deviation();
    
    if (!stm.line.see)
    {
        if (rtm.ball.have == 1)
        {
            robot_speed = 20.0f;
        
            if(rtm.goal.see)
            {
                robot_dir = (float)((rtm.goal.dir - 32) * 2);
            }
#if FORCE_LIMIT_ON == 1
            tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
            pid_motor_left.outMax = STANDARD_FORCE;
            pid_motor_left.outMin = -STANDARD_FORCE;
            pid_motor_right.outMax = STANDARD_FORCE;
            pid_motor_right.outMin = -STANDARD_FORCE;
            pid_motor_rear.outMax = STANDARD_FORCE;
            pid_motor_rear.outMin = -STANDARD_FORCE;
            tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
#endif
        }
        else
        {
            if (rtm.ball.see)
            {
                robot_speed = (float)speed_preset;
                robot_dir = (float)((rtm.ball.dir - 32) * 2);
#if FORCE_LIMIT_ON == 1
                tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
                pid_motor_left.outMax = STANDARD_FORCE;
                pid_motor_left.outMin = -STANDARD_FORCE;
                pid_motor_right.outMax = STANDARD_FORCE;
                pid_motor_right.outMin = -STANDARD_FORCE;
                pid_motor_rear.outMax = STANDARD_FORCE;
                pid_motor_rear.outMin = -STANDARD_FORCE;
                tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
#endif
            }
            else
            {
                robot_dir = 180.0f;
                robot_speed = 10.0f;
#if FORCE_LIMIT_ON == 1
                tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
                pid_motor_left.outMax = FORCE_LIMIT;
                pid_motor_left.outMin = -FORCE_LIMIT;
                pid_motor_right.outMax = FORCE_LIMIT;
                pid_motor_right.outMin = -FORCE_LIMIT;
                pid_motor_rear.outMax = FORCE_LIMIT;
                pid_motor_rear.outMin = -FORCE_LIMIT;
                tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
#endif
            }
        }        
    }        
    else
    {
        robot_dir = (float)(stm.line.esc - 180);
        robot_speed = 15.0f;
#if FORCE_LIMIT_ON == 1
        tc_disable_interrupt(TC0, 1, TC_IER_CPCS);
        pid_motor_left.outMax = STANDARD_FORCE;
        pid_motor_left.outMin = -STANDARD_FORCE;
        pid_motor_right.outMax = STANDARD_FORCE;
        pid_motor_right.outMin = -STANDARD_FORCE;
        pid_motor_rear.outMax = STANDARD_FORCE;
        pid_motor_rear.outMin = -STANDARD_FORCE;
        tc_enable_interrupt(TC0, 1, TC_IER_CPCS);
#endif
    }    

    robot_dir *= (3.14159265359f / 180.0f);
    
    mleft = robot_speed * (cos(robot_dir) * CosinMA1 - sin(robot_dir) * SinMA1);
    mright = robot_speed * (cos(robot_dir) * CosinMA2 - sin(robot_dir) * SinMA2);
    mrear = robot_speed * (cos(robot_dir) * CosinMA3 - sin(robot_dir) * SinMA3);
    
    mleft += compass_dev / 15.0f;
    mright += compass_dev / 15.0f;
    mrear += compass_dev / 15.0f;
    
    compensate_motor_output();
    
    set_motor_individual(mleft, mright, mrear);

    switch (event1)
    {
        case EVENT_BUTTON_RETURN_P:
            disable_motor();
            lcd_set_backlight(LCD_LIGHT_ON);
            lcd_clear();    // required to turn backlight on/off
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_match_voehringen(event_t event1)
{
    //float dir;
    //uint32_t ul_ticks_loop;
    //int16_t rel_deviation;
  
    //int8_t diff_x;
    //int8_t diff_y;
    
    float robot_speed = speed_preset;
    float robot_dir = 0;
    
    if(print_menu)
    {
        lcd_set_backlight(LCD_LIGHT_OFF);
        lcd_clear();    // required to turn backlight on/off
        
        if(inverted_start)
        {
            set_inverted_opponent_goal();
        }
        else
        {
            set_opponent_goal();
        }
        
        enable_motor();
    }
    
    estimate_rel_deviation();
    
    if (rtm.ball.have == 1)// && rtm.ball.dir >= 1 && rtm.ball.dir <= 63)
    {
        robot_speed = 20.0f;
        
        if(rtm.goal.see)
        {
            robot_dir = (rtm.goal.dir - 32) * 2;
        }
    }
    else
    {
        robot_dir = rtm.ball.dir - 32;
        
        if (rtm.ball.see)
        {
            robot_speed = (float)speed_preset;
            robot_dir *= 2.0f;
        }
        else
        {
            /*if(rtm.goal.see)
            {
                if((rtm.goal.dir - 32) > 5)
                {
                    robot_dir = 140;
                }
                else if((rtm.goal.dir - 32) < -5)
                {
                    robot_dir = -140;
                }
                else
                {
                    robot_dir = 180;
                }
            }
            else
            {*/
                robot_dir = 180;
            //}
            robot_speed = 10;
        }
    }
    
    if (stm.line.see == 1)
    {
        robot_dir = stm.line.esc - 180;
        robot_speed = 15.0f;
    }

    robot_dir *= (3.14159265359f / 180.0f);
    
    mleft = robot_speed * (cos(robot_dir) * CosinMA1 - sin(robot_dir) * SinMA1);
    mright = robot_speed * (cos(robot_dir) * CosinMA2 - sin(robot_dir) * SinMA2);
    mrear = robot_speed * (cos(robot_dir) * CosinMA3 - sin(robot_dir) * SinMA3);
    
    mleft += compass_dev / 15;
    mright += compass_dev / 15;
    mrear += compass_dev / 15;
    
    //compensate_motor_output(mleft, mright, mrear);
    
    set_motor_individual(mleft, mright, mrear);

    switch (event1)
    {
        case EVENT_BUTTON_RETURN_P:
            disable_motor();
            lcd_set_backlight(LCD_LIGHT_ON);
            lcd_clear();    // required to turn backlight on/off
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}
#endif
static void menu_sensors(event_t event1)
{
    if (print_menu)
    {
        print_menu_sensors();
    }
    
    switch(event1)
    {
        case EVENT_BUTTON_UP_P:
            if (menu_info.sensors.act_cursor_line > menu_info.sensors.min_cursor_line)
            {
                menu_info.sensors.act_cursor_line--;
                print_cursor(&menu_info.sensors);
            }
            break;
        case EVENT_BUTTON_DOWN_P:
            if (menu_info.sensors.act_cursor_line < menu_info.sensors.max_cursor_line)
            {
                menu_info.sensors.act_cursor_line++;
                print_cursor(&menu_info.sensors);
            }
            break;
        case EVENT_BUTTON_MID_P:
            switch (menu_info.sensors.act_cursor_line)
            {
                case 1:
                    act_menu = MENU_CAMERA;
                    print_menu = true;
                    break;
                case 2:
                    act_menu = MENU_COMPASS;
                    print_menu = true;
                    break;
                case 3:
                    act_menu = MENU_LINE;
                    print_menu = true;
                    break;
                case 4:
                    act_menu = MENU_ENCODER;
                    print_menu = true;
                    break;
                default:
                    break;
            }
            break;
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_camera(event_t event1)
{
    static Bool prev_rpi_inactive = false;
    static int8_t prev_ball_dir = 0;
    static Bool prev_ball_see = false;
    static Bool prev_ball_have = false;
    static int8_t prev_goal_dir = 0;
    static Bool prev_goal_see = false;
    static int8_t prev_goal_diff = 0;

    if(prev_rpi_inactive != s.rpi_inactive)
    {
        prev_rpi_inactive = s.rpi_inactive;
        print_menu = true;
    }

    if(print_menu)
    {
        lcd_clear();
    }
    
    if(!s.rpi_inactive)
    {
        if(s.ball.dir != prev_ball_dir || s.ball.see != prev_ball_see || print_menu)
        {
            if(s.ball.see)
            {
                sprintf(sprintf_buf, "Ball: %4d   ", s.ball.dir);
                lcd_print_s(1, 0, sprintf_buf);
            }
            else
            {
                lcd_print_s(1, 0, "no ball found");
            }

            prev_ball_dir = s.ball.dir;
            prev_ball_see = s.ball.see;
        }

        if(s.ball.have != prev_ball_have || print_menu)
        {
            sprintf(sprintf_buf, "Having ball: %1d %1d", s.ball.have, rtm.ball.have_2);
            lcd_print_s(2, 0, sprintf_buf);

            prev_ball_have = s.ball.have;
        }

        if(s.goal.dir != prev_goal_dir || s.goal.see != prev_goal_see || print_menu)
        {
            if (s.goal.see)
            {
                sprintf(sprintf_buf, "Goal: %4d   ", s.goal.dir);
                lcd_print_s(3, 0, sprintf_buf);
            }
            else
            {
                lcd_print_s(3, 0, "no goal found");
            }

            prev_goal_dir = s.goal.dir;
            prev_goal_see = s.goal.see;
        }
        
        if(s.goal.diff != prev_goal_diff || s.goal.see != prev_goal_see || print_menu)
        {
            if (s.goal.see)
            {
                sprintf(sprintf_buf, "1/2 width: %3d", s.goal.diff);
                lcd_print_s(4, 0, sprintf_buf);
            }
            else
            {
                lcd_print_s(4, 0, "no goal found ");
            }

            prev_goal_diff = s.goal.diff;
            prev_goal_see = s.goal.see;
        }
    }
    else
    {
        if(print_menu)
        {
            lcd_print_s(2, 4, "RPi inactive");
        }
    }

#if 0
    if(rtm.ball.dir != prev_ball_dir || rtm.ball.see != prev_ball_see || print_menu)
    {
        if (rtm.ball.dir == 0)
        {
            lcd_print_s(1, 0, "RPi inactive ");
        }
        else if (rtm.ball.see) //  && rtm.ball.dir != 0
        {
            sprintf(sprintf_buf, "Ball: %4d   ", (rtm.ball.dir - 32) * 2);
            lcd_print_s(1, 0, sprintf_buf);
        }
        else
        {
            lcd_print_s(1, 0, "no ball found");
        }
        prev_ball_dir = rtm.ball.dir;
        prev_ball_see = rtm.ball.see;
    }
    
    if(rtm.ball.have != prev_ball_have || print_menu)
    {
        sprintf(sprintf_buf, "Having ball: %1d", rtm.ball.have);
        lcd_print_s(2, 0, sprintf_buf);
        prev_ball_have = rtm.ball.have;
    }
    
    if(rtm.goal.dir != prev_goal_dir || rtm.goal.see != prev_goal_see || print_menu)
    {
        if (rtm.goal.dir == 0)
        {
            lcd_print_s(3, 0, "RPi inactive ");
        }
        else if (rtm.goal.see)
        {
            sprintf(sprintf_buf, "Goal: %4d   ", (rtm.goal.dir - 32) * 2);
            lcd_print_s(3, 0, sprintf_buf);
        }
        else
        {
            lcd_print_s(3, 0, "no goal found");
        }
        prev_goal_dir = rtm.goal.dir;
        prev_goal_see = rtm.goal.see;
    }
    
    if(rtm.goal.diff != prev_goal_diff || rtm.goal.see != prev_goal_see || print_menu)
    {
        if (rtm.goal.dir == 0)
        {
            lcd_print_s(4, 0, "RPi inactive  ");
        }
        else if (rtm.goal.see)
        {
            sprintf(sprintf_buf, "1/2 width: %3d", rtm.goal.diff * 2);
            lcd_print_s(4, 0, sprintf_buf);
        }
        else
        {
            lcd_print_s(4, 0, "no goal found ");
        }
        prev_goal_diff = rtm.goal.diff;
        prev_goal_see = rtm.goal.see;
    }
#endif
    switch (event1)
    {
        case EVENT_BUTTON_RETURN_P:
        	act_menu = MENU_SENSORS;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_compass(event_t event1)
{
    static uint16_t prev_direction = 0;
    
    if(print_menu)
    {
        lcd_clear();
        lcd_print_s(4, 2, "press mid to cal");
    }
    
    update_compass();
    if(direction != prev_direction)
    {
        prev_direction = direction;
        sprintf(sprintf_buf, "  Direction: %3.1f  ", (float)direction / 10.0);
        lcd_print_s(2, 0, sprintf_buf);
    }
    
    switch(event1)
    {
        case EVENT_BUTTON_MID_P:
            act_menu = MENU_COMPASS_CALIBRATION;
            print_menu = true;
            break;
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_SENSORS;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_compass_calibration(event_t event1)
{
    uint8_t compass_cal_step = 0;

    if(print_menu)
    {
        lcd_clear();
        lcd_print_s(2, 1, "calibrate compass");
        sprintf(sprintf_buf, "  Direction: %1d  ", compass_cal_step + 1);
        lcd_print_s(3, 1, sprintf_buf);
    }
    
    if(event1 == EVENT_BUTTON_MID_P)
    {
        twi_packet_t *tx_packet = twi_get_tx_packet();
        
        tx_packet->chip = 0x60;
        tx_packet->addr[0] = 0x0f;
        tx_packet->addr_length = 1;
        
        tx_packet->buffer[0] = 0xff;
        tx_packet->length = 1;
        
        set_compass_is_busy();
        twi_pdc_master_write(TWI0, tx_packet);
        while(compass_is_busy());
        mdelay(500);
        
        compass_cal_step++;
        
        if(compass_cal_step == 4)
        {
            compass_cal_step = 0;
            act_menu = MENU_COMPASS;
            print_menu = true;
        }
        print_menu = true;
    }
    
    switch (event1)
    {
        case EVENT_BUTTON_MID_P:
            /*twi_packet_t *tx_packet = twi_get_tx_packet();
            
            tx_packet->chip = 0x60;
            tx_packet->addr[0] = 0x0f;
            tx_packet->addr_length = 1;
            
            tx_packet->buffer[0] = 0xff;
            tx_packet->length = 1;
            
            set_compass_is_busy();
            twi_pdc_master_write(TWI0, tx_packet);
            while(compass_is_busy());
            mdelay(500);
            
            compass_cal_step++;
            
            if(compass_cal_step == 4)
            {
                compass_cal_step = 0;
                act_menu = MENU_COMPASS;
                print_menu = true;
            }
            print_menu = true;*/
            break;
        case EVENT_BUTTON_RETURN_P:
            if(compass_cal_step == 0)
            {
                act_menu = MENU_COMPASS;
                print_menu = true;
            }
            else
            {
                print_menu = false;
            }
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_line(event_t event1)
{
    static uint16_t prev_line_values;

    if(print_menu)
    {
        lcd_clear();
    }

    if(stm.line.all != prev_line_values || print_menu)
    {
        sprintf(sprintf_buf, "See: %1d", stm.line.see);
        lcd_print_s(1, 0, sprintf_buf);
        sprintf(sprintf_buf, "Esc: %4d", stm.line.esc);
        lcd_print_s(2, 0, sprintf_buf);
        sprintf(sprintf_buf, "Line: %1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d", stm.line.single.segment_1, stm.line.single.segment_2,
        stm.line.single.segment_3, stm.line.single.segment_4, stm.line.single.segment_5, stm.line.single.segment_6, stm.line.single.segment_7,
        stm.line.single.segment_8, stm.line.single.segment_9, stm.line.single.segment_10, stm.line.single.segment_11, stm.line.single.segment_12);
        lcd_print_s(3, 0, sprintf_buf);
        prev_line_values = stm.line.all;
        lcd_print_s(4, 1, "press mid to cal");
    }

    switch(event1)
    {
        case EVENT_BUTTON_MID_P:
            act_menu = MENU_LINE_CALIBRATION;
            print_menu = true;
            break;
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_SENSORS;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_line_calibration(event_t event1)
{
    if(print_menu)
    {
        lcd_clear();
        sprintf(sprintf_buf, "Cal value: %2d", mts.line_cal_value);
        lcd_print_s(2, 0, sprintf_buf);
    }

    switch (event1)
    {
        case EVENT_BUTTON_LEFT_P:
            if(mts.line_cal_value > 0)
            {
                mts.line_cal_value--;
                ini_update_line_cal = true;
                print_menu = true;
            }
            break;
        case EVENT_BUTTON_RIGHT_P:
            if(mts.line_cal_value < 16)
            {
                mts.line_cal_value++;
                ini_update_line_cal = true;
                print_menu = true;
            }
            break;
        case EVENT_BUTTON_RETURN_P:
            if(ini_update_line_cal)
            {
                ini_update_line_cal = false;
                if (f_open(&noOS_ini_file, "noOS.ini", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
                {
                    sprintf(str, "%2d", mts.line_cal_value);
                    iniparser_set(noOS_ini_dict, "general:line_cal", str);
                    iniparser_dump_ini(noOS_ini_dict, &noOS_ini_file);
                    f_close(&noOS_ini_file);
                }
                else
                {
                    // todo error menu
                }
            }
            act_menu = MENU_LINE;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_encoder(event_t event1)
{
    static Bool updated_ref = true;

    static int16_t ref_motor_speed_left = 0;
    static int16_t ref_motor_speed_right = 0;
    static int16_t ref_motor_speed_rear = 0;
    static int16_t prev_motor_speed_left = 0;
    static int16_t prev_motor_speed_right = 0;
    static int16_t prev_motor_speed_rear = 0;
    
    if(print_menu)
    {
        enable_motor();
        lcd_clear();
        print_cursor(&menu_info.encoder);
    }
    
    int16_t tmp_motor_speed_left = (int16_t)(act_motor_speed_left + 0.5);
    
    if(prev_motor_speed_left != tmp_motor_speed_left || updated_ref || print_menu)
    {
        prev_motor_speed_left = tmp_motor_speed_left;
        sprintf(sprintf_buf, "Left: %3d  %3d", tmp_motor_speed_left, ref_motor_speed_left);
        lcd_print_s(1, 1, sprintf_buf);
    }
    
    int16_t tmp_motor_speed_right = (int16_t)(act_motor_speed_right + 0.5);

    if(prev_motor_speed_right != tmp_motor_speed_right || updated_ref || print_menu)
    {
        prev_motor_speed_right = tmp_motor_speed_right;
        sprintf(sprintf_buf, "Right: %3d  %3d", tmp_motor_speed_right, ref_motor_speed_right);
        lcd_print_s(2, 1, sprintf_buf);
    }
    
    int16_t tmp_motor_speed_rear = (int16_t)(act_motor_speed_rear + 0.5);

    if(prev_motor_speed_rear != tmp_motor_speed_rear || updated_ref || print_menu)
    {
        prev_motor_speed_rear =tmp_motor_speed_rear;
        sprintf(sprintf_buf, "Rear: %3d  %3d", tmp_motor_speed_rear, ref_motor_speed_rear);
        lcd_print_s(3, 1, sprintf_buf);
    }
    
    set_motor_individual(ref_motor_speed_left, ref_motor_speed_right, ref_motor_speed_rear);

    switch(event1)
    {
        case EVENT_BUTTON_UP_P:
            if (menu_info.encoder.act_cursor_line > menu_info.encoder.min_cursor_line)
            {
                menu_info.encoder.act_cursor_line--;
                print_cursor(&menu_info.encoder);
            }
            break;
        case EVENT_BUTTON_DOWN_P:
            if (menu_info.encoder.act_cursor_line < menu_info.encoder.max_cursor_line)
            {
                menu_info.encoder.act_cursor_line++;
                print_cursor(&menu_info.encoder);
            }
            break;
        case EVENT_BUTTON_LEFT_P:
            switch(menu_info.encoder.act_cursor_line)
            {
                case 1:
                    if (ref_motor_speed_left > -MAX_MOTOR_SPEED)
                    {
                        ref_motor_speed_left -= 2;
                        updated_ref = true;
                    }
                    break;
                case 2:
                    if (ref_motor_speed_right > -MAX_MOTOR_SPEED)
                    {
                        ref_motor_speed_right -= 2;
                        updated_ref = true;
                    }
                    break;
                case 3:
                    if (ref_motor_speed_rear > -MAX_MOTOR_SPEED)
                    {
                        ref_motor_speed_rear -= 2;
                        updated_ref = true;
                    }
                    break;
                default:
                    break;
            }
            break;
        case EVENT_BUTTON_RIGHT_P:
            switch(menu_info.encoder.act_cursor_line)
            {
                case 1:
                if (ref_motor_speed_left < MAX_MOTOR_SPEED)
                {
                    ref_motor_speed_left += 2;
                    updated_ref = true;
                }
                break;
                case 2:
                if (ref_motor_speed_right < MAX_MOTOR_SPEED)
                {
                    ref_motor_speed_right += 2;
                    updated_ref = true;
                }
                break;
                case 3:
                if (ref_motor_speed_rear < MAX_MOTOR_SPEED)
                {
                    ref_motor_speed_rear += 2;
                    updated_ref = true;
                }
                break;
                default:
                break;
            }
            break;
        case EVENT_BUTTON_RETURN_P:
            ref_motor_speed_left = 0;
            ref_motor_speed_right = 0;
            ref_motor_speed_rear = 0;
            set_motor_individual(0, 0, 0);
            disable_motor();
            act_menu = MENU_SENSORS;
            print_menu = true;
            break;
        default:
            updated_ref = false;
            print_menu = false;
            break;
    }
}

static void menu_settings(event_t event1)
{
    if (print_menu)
    {
        print_menu_settings();
    }
    
    switch(event1)
    {
        case EVENT_BUTTON_UP_P:
            if (menu_info.settings.act_cursor_line > menu_info.settings.min_cursor_line)
            {
                menu_info.settings.act_cursor_line--;
                print_cursor(&menu_info.settings);
            }
            break;
        case EVENT_BUTTON_DOWN_P:
            if (menu_info.settings.act_cursor_line < menu_info.settings.max_cursor_line)
            {
                menu_info.settings.act_cursor_line++;
                print_cursor(&menu_info.settings);
            }
            break;
        case EVENT_BUTTON_LEFT_P:
            switch (menu_info.settings.act_cursor_line)
            {
                case 1:
                    if(robot_id > 1)
                    {
                        robot_id--;
                        ini_update_robot_id = true;
                        print_menu = true;
                    }
                    break;
                case 2:
                    if(speed_preset > 4)
                    {
                        speed_preset -= 5;
                        ini_update_speed = true;
                        print_menu = true;
                    }
                    break;
                case 3:
                    if(heartbeat)
                    {
                        heartbeat = false;
                        ioport_set_pin_level(LED_ONBOARD, 0);
                        ioport_set_pin_level(LED_M1, 0);
                        mts.ibit.heartbeat = 0;
                        ini_update_heartbeat = true;
                        print_menu = true;
                    }
                    break;
                default:
                    break;
            }
            break;
        case EVENT_BUTTON_RIGHT_P:
            switch (menu_info.settings.act_cursor_line)
            {
                case 1:
                    if(robot_id < 2)
                    {
                        robot_id++;
                        ini_update_robot_id = true;
                        print_menu = true;
                    }
                    break;
                case 2:
                    if(speed_preset < MAX_MOTOR_SPEED)
                    {
                        speed_preset += 5;
                        ini_update_speed = true;
                        print_menu = true;
                    }
                    break;
                case 3:
                    if(!heartbeat)
                    {
                        heartbeat = true;
                        ini_update_heartbeat = true;
                        print_menu = true;
                    }
                    break;
                default:
                    break;
            }
            break;
        /*case EVENT_BUTTON_MID_P:
            switch (menu_info.settings.act_cursor_line)
            {
                case 1:
                    
                    break;
                case 2:
                    
                    break;
                case 3:
                    
                    break;
                default:
                    break;
            }
            break;*/
        case EVENT_BUTTON_RETURN_P:
            if(ini_update_robot_id)
            {
                ini_update_robot_id = false;
                if (f_open(&noOS_ini_file, "noOS.ini", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
                {
                    sprintf(str, "%d", robot_id);
                    iniparser_set(noOS_ini_dict, "general:robot_id", str);
                    iniparser_dump_ini(noOS_ini_dict, &noOS_ini_file);
                    f_close(&noOS_ini_file);
                }
                else
                {
                    // todo error menu
                }
            }
            if(ini_update_speed)
            {
                ini_update_speed = false;
                if (f_open(&noOS_ini_file, "noOS.ini", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
                {
                    sprintf(str, "%3d", speed_preset);
                    iniparser_set(noOS_ini_dict, "general:speed", str);
                    iniparser_dump_ini(noOS_ini_dict, &noOS_ini_file);
                    f_close(&noOS_ini_file);
                }
                else
                {
                    // todo error menu
                }
            }
            if(ini_update_heartbeat)
            {
                ini_update_heartbeat = false;
                if (f_open(&noOS_ini_file, "noOS.ini", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
                {
                    sprintf(str, "%d", heartbeat);
                    iniparser_set(noOS_ini_dict, "general:heartbeat", str);
                    iniparser_dump_ini(noOS_ini_dict, &noOS_ini_file);
                    f_close(&noOS_ini_file);
                }
                else
                {
                    // todo error menu
                }
            }
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_bootup(event_t event1)
{
    if (print_menu)
    {
        for(int i = 0; i< 3; i++)
        {
            ioport_set_pin_level(LED_ONBOARD, 1);
            ioport_set_pin_level(LED_BAT, 1);
            ioport_set_pin_level(LED_M1, 1);
            ioport_set_pin_level(LED_M2, 1);
            ioport_set_pin_level(LED_M3, 1);
            mdelay(100);
            ioport_set_pin_level(LED_ONBOARD, 0);
            ioport_set_pin_level(LED_BAT, 0);
            ioport_set_pin_level(LED_M1, 0);
            ioport_set_pin_level(LED_M2, 0);
            ioport_set_pin_level(LED_M3, 0);
            mdelay(100);
        }

        print_menu = false;
    }

    if (getTicks() >= (ticks_dot_update + 500))
    {
        ticks_dot_update = getTicks();
            
        if (dots < 3)
        {
            dots++;
        }
        else
        {
            dots = 0;
        }
            
        update_dots = 1;
    }
        
    if (update_dots)
    {
        update_dots = 0;
            
        switch (dots)
        {
            case 0:
                lcd_print_s(2, 2, "booting noOS   ");
                break;
            case 1:
                lcd_print_s(2, 14, ".");
                break;
            case 2:
                lcd_print_s(2, 15, ".");
                break;
            case 3:
                lcd_print_s(2, 16, ".");
                break;
            default:
                break;
        }
    }
    
    if(ioport_get_pin_level(RPI2) || event1 == EVENT_BUTTON_MID_P)
    {
        act_menu = MENU_MAIN;
        print_menu = true;
    }
}

static void menu_shutdown(event_t event1)
{
    static uint32_t ticks_shutdown;

    if(shutdown_confirmed)
    {
        lcd_clear();
        lcd_print_s(2, 2, "shutting down...");
        
        ioport_set_pin_level(LED_ONBOARD, 0);
        ioport_set_pin_level(LED_BAT, 0);
        ioport_set_pin_level(LED_M1, 0);
        ioport_set_pin_level(LED_M2, 0);
        ioport_set_pin_level(LED_M3, 0);
        
        pwm_channel_disable(PWM, MOTOR_LEFT);
        pwm_channel_disable(PWM, MOTOR_RIGHT);
        pwm_channel_disable(PWM, MOTOR_REAR);
        pwm_channel_disable(PWM, ENC_CLK);
        
        //sensor_parameters.ibit.sleep_mode = 1;
        update_comm();
        
        ioport_set_pin_level(RPI1, 0);

        while (ioport_get_pin_level(RPI2) == 1)
        {
            update_comm();
            check_battery();
        }

        ticks_shutdown = getTicks();

        while((getTicks() - ticks_shutdown) < 7500)
        {
            update_comm();
            check_battery();
        }
        
        lcd_set_backlight(LCD_LIGHT_OFF);
        lcd_clear();    // required to turn backlight on/off
        mdelay(100);
        lcd_set_backlight(LCD_LIGHT_ON);
        lcd_clear();    // required to turn backlight on/off
        mdelay(100);
        lcd_set_backlight(LCD_LIGHT_OFF);
        lcd_clear();    // required to turn backlight on/off
        
        while(1)
        {
            update_comm();
            check_battery();
        }
    }
    else
    {
        if(print_menu)
        {
            lcd_clear();
            lcd_print_s(2, 1, "confirm shutdown?");
        }
    }
    
    switch (event1)
    {
        case EVENT_BUTTON_MID_P:
            shutdown_confirmed = 1;
            break;
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void print_menu_main(void)
{
    const char* text[4] = {"   no robot id", " Match", " Sensors", " Settings"};

    if(robot_id == 1)
    {
        text[0] = "    noOS ONE";
    }
    else if(robot_id == 2)
    {
        text[0] = "    noOS TWO";
    }
    
//    lcd_print_m(text);
    lcd_clear();
    lcd_print_s(1, 0, text[0]);
    lcd_print_s(2, 0, text[1]);
    lcd_print_s(3, 0, text[2]);
    lcd_print_s(4, 0, text[3]);
    print_cursor(&menu_info.main);
    update_battery(1);
}

static void print_menu_sensors(void)
{
    const char* text[4] = {" Camera", " Compass", " Line", " Encoder"};
//    lcd_print_m(text);
    lcd_clear();
    lcd_print_s(1, 0, text[0]);
    lcd_print_s(2, 0, text[1]);
    lcd_print_s(3, 0, text[2]);
    lcd_print_s(4, 0, text[3]);
    print_cursor(&menu_info.sensors);
}

static void print_menu_settings(void)
{
    const char* text[4] = {" ", " ", " ", " "};
    
    lcd_clear();

    sprintf(sprintf_buf, " Robot id: %1d", robot_id);
    text[0] = sprintf_buf;
    lcd_print_s(1, 0, text[0]);

    sprintf(sprintf_buf, " Speed: %3d", speed_preset);
    text[1] = sprintf_buf;
    lcd_print_s(2, 0, text[1]);

    sprintf(sprintf_buf, " Heartbeat: %d", heartbeat);
    text[2] = sprintf_buf;
    lcd_print_s(3, 0, text[2]);

    lcd_print_s(4, 0, text[3]);

    print_cursor(&menu_info.settings);
}

static void print_cursor(menu_info_t *info)
{
    lcd_print_s(info->prev_cursor_line, 0, " ");
    lcd_print_s(info->act_cursor_line, 0, ">");
    
    info->prev_cursor_line = info->act_cursor_line;
}

event_t button_events(void)
{
    event_t nextEvent = EVENT_NO_EVENT;

    if (getTicks() >= (ticks_button_update + 30))
    {
        ticks_button_update = getTicks();
        
        pb_up_act = ioport_get_pin_level(PB_UP);
        pb_left_act = ioport_get_pin_level(PB_LEFT);
        pb_mid_act = ioport_get_pin_level(PB_MID);
        pb_right_act = ioport_get_pin_level(PB_RIGHT);
        pb_down_act = ioport_get_pin_level(PB_DOWN);
        pb_return_act = ioport_get_pin_level(PB_RETURN);
        
        if (pb_up_act != pb_up_prev && pb_up_act == 0)
        {
            nextEvent = EVENT_BUTTON_UP_P;
        }
        else if (pb_up_act != pb_up_prev && pb_up_act == 1)
        {
            nextEvent = EVENT_BUTTON_UP_R;
        }
        /*else if (pb_up_act == pb_up_prev && pb_up_act == 0)
        {
            nextEvent = EVENT_BUTTON_UP_H;
        }*/
        
        if (pb_left_act != pb_left_prev && pb_left_act == 0)
        {
            nextEvent = EVENT_BUTTON_LEFT_P;
        }
        else if (pb_left_act != pb_left_prev && pb_left_act == 1)
        {
            nextEvent = EVENT_BUTTON_LEFT_R;
        }
        /*else if (pb_left_act == pb_left_prev && pb_left_act == 0)
        {
            nextEvent = EVENT_BUTTON_LEFT_H;
        }*/
        
        if (pb_mid_act != pb_mid_prev && pb_mid_act == 0)
        {
            nextEvent = EVENT_BUTTON_MID_P;
        }
        else if (pb_mid_act != pb_mid_prev && pb_mid_act == 1)
        {
            nextEvent = EVENT_BUTTON_MID_R;
        }
        /*else if (pb_mid_act == pb_mid_prev && pb_mid_act == 0)
        {
            nextEvent = EVENT_BUTTON_MID_H;
        }*/
        
        if (pb_right_act != pb_right_prev && pb_right_act == 0)
        {
            nextEvent = EVENT_BUTTON_RIGHT_P;
        }
        else if (pb_right_act != pb_right_prev && pb_right_act == 1)
        {
            nextEvent = EVENT_BUTTON_RIGHT_R;
        }
        /*else if (pb_right_act == pb_right_prev && pb_right_act == 0)
        {
            nextEvent = EVENT_BUTTON_RIGHT_H;
        }*/
        
        if (pb_down_act != pb_down_prev && pb_down_act == 0)
        {
            nextEvent = EVENT_BUTTON_DOWN_P;
        }
        else if (pb_down_act != pb_down_prev && pb_down_act == 1)
        {
            nextEvent = EVENT_BUTTON_DOWN_R;
        }
        /*else if (pb_down_act == pb_down_prev && pb_down_act == 0)
        {
            nextEvent = EVENT_BUTTON_DOWN_H;
        }*/
        
        if (pb_return_act != pb_return_prev && pb_return_act == 0)
        {
            nextEvent = EVENT_BUTTON_RETURN_P;
        }
        else if (pb_return_act != pb_return_prev && pb_return_act == 1)
        {
            nextEvent = EVENT_BUTTON_RETURN_R;
        }
        /*else if (pb_return_act == pb_up_prev && pb_return_act == 0)
        {
            nextEvent = EVENT_BUTTON_RETURN_H;
        }*/
        
        pb_up_prev = pb_up_act;
        pb_left_prev = pb_left_act;
        pb_mid_prev = pb_mid_act;
        pb_right_prev = pb_right_act;
        pb_down_prev = pb_down_act;
        pb_return_prev = pb_return_act;
    }

    return nextEvent;
}
