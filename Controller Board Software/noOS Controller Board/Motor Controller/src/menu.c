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

#define SPEED_FAST      70
#define SPEED_SLOW      30

#define TURN_NONE       0
#define TURN_COMPASS    1
#define TURN_GOAL       2

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
    menu_info_t pid_tuner;
} menu_info =
  {
      { 2, 2, 2, 4 },
      { 1, 1, 1, 4 },
      { 1, 1, 1, 3 },
      { 1, 1, 1, 3 },
      { 1, 1, 1, 4 }
  };

Bool shutdown_confirmed = 0;
Bool inverted_start = false;

char sprintf_buf[21];

static void menu_main(event_t event1);
static void menu_match_hannover(event_t event1);
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
            menu_match_hannover(event1);
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

static void menu_test(event_t event1)
{
    static uint32_t ticks_test = 0;
    static pidReg_t pid_turn;
    static Bool pid_updated = true;
    static uint8_t turn_target = TURN_NONE;
    
    if(print_menu)
    {
        pid_turn.kp = 0.0f;
        pid_turn.ki = 0.0f;
        pid_turn.kc = 0.0f;
        pid_turn.kd = 0.0f;
        pid_turn.outMin = -150.0f;
        pid_turn.outMax = 150.0f;
        pid_turn.intg = 0.0f;
        pid_turn.prevErr = 0.0f;
        pid_turn.satErr = 0.0f;
        
        pid_updated = true;
        
        //lcd_set_backlight(LCD_LIGHT_OFF);
        lcd_clear(); // required to turn backlight on/off
        
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
    
    if ((getTicks() - ticks_test) > 100)
    {
        ticks_test = getTicks();
        
        estimate_rel_deviation();
        
        float robot_trn = 0;
        
        if(turn_target == TURN_COMPASS)
        {
            robot_trn = pidReg_compass(&pid_turn, 0, -s.compass);
        }
        else if(turn_target == TURN_GOAL)
        {
            robot_trn = pidReg_compass(&pid_turn, 0, s.goal.dir);
        }
        
        set_motor(0, 0, robot_trn);
    }
    
    if(pid_updated)
    {
        pid_updated = false;
        sprintf(sprintf_buf, "P: %2.1f", pid_turn.kp);
        lcd_print_s(1, 1, sprintf_buf);
        sprintf(sprintf_buf, "I: %1.2f", pid_turn.ki);
        lcd_print_s(2, 1, sprintf_buf);
        sprintf(sprintf_buf, "C: %2.1f", pid_turn.kc);
        lcd_print_s(3, 1, sprintf_buf);
        sprintf(sprintf_buf, "D: %2.1f", pid_turn.kd);
        lcd_print_s(4, 1, sprintf_buf);
        print_cursor(&menu_info.pid_tuner);
    }
    
    switch (event1)
    {
        case EVENT_BUTTON_MID_P:
            if(turn_target == TURN_COMPASS)
            {
                turn_target = TURN_GOAL;
                ioport_set_pin_level(LED_M1, 0);
            }
            else
            {
                turn_target = TURN_COMPASS;
                ioport_set_pin_level(LED_M1, 1);
            }
            break;
        case EVENT_BUTTON_UP_P:
            if (menu_info.pid_tuner.act_cursor_line > menu_info.pid_tuner.min_cursor_line)
            {
                menu_info.pid_tuner.act_cursor_line--;
                print_cursor(&menu_info.pid_tuner);
            }
            break;
        case EVENT_BUTTON_DOWN_P:
            if (menu_info.pid_tuner.act_cursor_line < menu_info.pid_tuner.max_cursor_line)
            {
                menu_info.pid_tuner.act_cursor_line++;
                print_cursor(&menu_info.pid_tuner);
            }
            break;
        case EVENT_BUTTON_LEFT_P:
            switch (menu_info.pid_tuner.act_cursor_line)
            {
                case 1:
                    if(pid_turn.kp >= 0.1f)
                    {
                        pid_turn.kp -= 0.1f;
                        pid_updated = true;
                    }
                    else
                    {
                        pid_turn.kp = 0.0f;
                        pid_updated = true;
                    }
                    break;
                case 2:
                    if(pid_turn.ki >= 0.01f)
                    {
                        pid_turn.ki -= 0.01f;
                        pid_updated = true;
                    }
                    else
                    {
                        pid_turn.ki = 0.0f;
                        pid_updated = true;
                    }
                    break;
                case 3:
                    if(pid_turn.kc >= 0.1f)
                    {
                        pid_turn.kc -= 0.1f;
                        pid_updated = true;
                    }
                    else
                    {
                        pid_turn.kc = 0.0f;
                        pid_updated = true;
                    }
                    break;
                case 4:
                    if(pid_turn.kd >= 0.1f)
                    {
                        pid_turn.kd -= 0.1f;
                        pid_updated = true;
                    }
                    else
                    {
                        pid_turn.kd = 0.0f;
                        pid_updated = true;
                    }
                    break;
                default:
                    break;
            }
            break;
        case EVENT_BUTTON_RIGHT_P:
            switch (menu_info.pid_tuner.act_cursor_line)
            {
                case 1:
                    if(pid_turn.kp < 10)
                    {
                        pid_turn.kp += 0.1f;
                        pid_updated = true;
                    }
                    break;
                case 2:
                    if(pid_turn.ki < 10)
                    {
                        pid_turn.ki += 0.1f;
                        pid_updated = true;
                    }
                    break;
                case 3:
                    if(pid_turn.kc < 10)
                    {
                        pid_turn.kc += 0.1f;
                        pid_updated = true;
                    }
                    break;
                case 4:
                    if(pid_turn.kd < 10)
                    {
                        pid_turn.kd += 0.1f;
                        pid_updated = true;
                    }
                    break;
                default:
                    break;
            }
            break;
        case EVENT_BUTTON_RETURN_P:
            disable_motor();
            lcd_set_backlight(LCD_LIGHT_ON);
            lcd_clear(); // required to turn backlight on/off
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_match_hannover(event_t event1)
{
    static pidReg_t pid_compass;
    static float pid_compass_out = 0.0f;
    static uint8_t turn_target = TURN_NONE;
    
    static Bool arrived_rear = false;
    float robot_speed = 0.0f;
    float robot_dir = 0.0f;
    float robot_trn = 0.0f;
    
    if(print_menu)
    {
        pid_compass.kp = 0.7f;
        pid_compass.ki = 0.0f;
        pid_compass.kc = 0.0f;
        pid_compass.kd = 0.6f;
        pid_compass.outMin = -150.0f;
        pid_compass.outMax = 150.0f;
        pid_compass.intg = 0.0f;
        pid_compass.prevErr = 0.0f;
        pid_compass.satErr = 0.0f;
        
        lcd_set_backlight(LCD_LIGHT_OFF);
        lcd_clear(); //required to turn backlight on/off
        
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
    
    if(update_pid_compass)
    {
        update_pid_compass = false;
        pid_compass_out = pidReg_compass(&pid_compass, 0, -s.compass);
    }
    
    ioport_set_pin_level(LED_M1, 0);
    ioport_set_pin_level(LED_M2, 0);
    //ioport_set_pin_level(LED_M3, 0);
    
    if(!s.ball.see && ((s.distance.one.arrived && robot_id == 1) || (s.distance.two.arrived && robot_id == 2)))//((s.line.see && s.line.esc < 45 && s.line.esc > -45) || s.distance))
    {
        arrived_rear = true;
    }
    else// if(s.ball.see)
    {
        arrived_rear = false;
    }        
    
    turn_target = TURN_COMPASS;
    //robot_trn = s.compass;

    if(s.ball.have || s.ball.have_2)
    {
        robot_speed = 100.0f;
            
        if(s.goal.see)
        {
            if(abs(s.goal.dir) > s.goal.diff)
            {
                robot_speed = 50.0f;
            }
            
            turn_target = TURN_GOAL;
            //robot_trn = -s.goal.dir;
        }
    }
    else
    {
        if(s.ball.see)
        {
            robot_dir = (float)s.ball.dir * 2.2f;
            robot_speed = 75.0f;
        }
        else
        {
            if(!arrived_rear)
            {
                robot_speed = 75.0f;
                
                if((robot_id == 1 && !s.distance.one.arrived && !s.distance.one.correction_dir)\
                || (robot_id == 2 && !s.distance.two.arrived && !s.distance.two.correction_dir))
                {
                    if(s.goal.see && (s.goal.dir < -10 || s.goal.dir > 10))
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
                /*if(abs(s.goal.dir) >= 3)
                {
                    robot_speed = abs(s.goal.dir) * 2;
                    
                    if(s.goal.dir >= 0)
                    {
                        robot_dir = 90.0f;
                    }
                    else
                    {
                        robot_dir = -90.0f;
                    }
                }
                else
                {
                    
                }*/
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
            /*if(abs(robot_dir - s.line.esc) > 180)
            {
                if(s.line.esc > robot_dir)
                {
                    robot_dir += 360;
                }
                else
                {
                    robot_dir -= 360;
                }
            }*/
            
            if(abs(esc_dir_diff) <= 90)
            {
                if(abs(robot_dir) < abs(esc_min))
                {
                    robot_dir = esc_min;
                    ioport_set_pin_level(LED_M2, 1);
                }
                if(abs(robot_dir) > abs(esc_max))
                {
                    robot_dir = esc_max;
                    ioport_set_pin_level(LED_M1, 1);
                }
            }
            else
            {
                //robot_dir = robot_dir < 0 ? esc_min : esc_max;
                
                if(abs(abs(robot_dir) - abs(esc_min)) > abs(abs(robot_dir) - abs(esc_max)))
                {
                    robot_dir = esc_max;
                }
                else
                {
                    robot_dir = esc_min;
                }
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
        //pid_compass_out = pidReg_compass(&pid_compass, 0, s.goal.dir);
    }
    else
    {
        robot_trn = 0.0f;
    }
    
    set_motor(robot_speed, robot_dir, robot_trn);

    switch (event1)
    {
        case EVENT_BUTTON_RETURN_P:
            disable_motor();
            ioport_set_pin_level(LED_M1, 0);
            ioport_set_pin_level(LED_M2, 0);
            ioport_set_pin_level(LED_M3, 0);
            lcd_set_backlight(LCD_LIGHT_ON);
            lcd_clear(); //required to turn backlight on/off
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

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
    static Bool prev_ball_have_2 = false;
    static int8_t prev_goal_dir = 0;
    static Bool prev_goal_see = false;
    static int8_t prev_goal_diff = 0;
    static uint8_t prev_camera_fps = 0;

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

        if(s.ball.have != prev_ball_have || s.ball.have_2 != prev_ball_have_2 || print_menu)
        {
            sprintf(sprintf_buf, "Having ball: %1d %1d", s.ball.have, s.ball.have_2);
            lcd_print_s(2, 0, sprintf_buf);

            prev_ball_have = s.ball.have;
            prev_ball_have_2 = s.ball.have_2;
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
        
        if(s.camera_fps != prev_camera_fps || print_menu)
        {
            sprintf(sprintf_buf, "FPS:%2d", s.camera_fps);
            lcd_print_s(1, 14, sprintf_buf);
            prev_camera_fps = s.camera_fps;
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
    static float prev_direction = 0.1f;
    static uint16_t prev_raw_direction = 1;
    
    if(print_menu)
    {
        lcd_clear();
        lcd_print_s(4, 2, "press mid to cal");
        set_opponent_goal();
    }
    
    estimate_rel_deviation();
    
    if(direction != prev_raw_direction)
    {
        prev_raw_direction = direction;
        sprintf(sprintf_buf, "  Direction: %4.1f  ", ((float)direction / 10));
        lcd_print_s(1, 0, sprintf_buf);
    }
    
    if((int16_t)s.compass != (int16_t)prev_direction)
    {
        prev_direction = s.compass;
        sprintf(sprintf_buf, "  Direction: %4.1f  ", s.compass);
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
    static Bool prev_line_see = false;

    if(print_menu)
    {
        lcd_clear();
    }

    if(s.line.all != prev_line_values || s.line.see != prev_line_see || print_menu)
    {
        sprintf(sprintf_buf, "See: %1d", s.line.see);
        lcd_print_s(1, 0, sprintf_buf);
        sprintf(sprintf_buf, "Esc: %4d  Diff: %3d", s.line.esc, s.line.diff);
        lcd_print_s(2, 0, sprintf_buf);
        sprintf(sprintf_buf, "Line: %1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d", s.line.single.segment_1, s.line.single.segment_2,
        s.line.single.segment_3, s.line.single.segment_4, s.line.single.segment_5, s.line.single.segment_6, s.line.single.segment_7,
        s.line.single.segment_8, s.line.single.segment_9, s.line.single.segment_10, s.line.single.segment_11, s.line.single.segment_12);
        lcd_print_s(3, 0, sprintf_buf);
        prev_line_values = s.line.all;
        prev_line_see = s.line.see;
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
