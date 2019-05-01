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

menu_t act_menu = MENU_MAIN;
Bool print_menu = true;

uint8_t prev_ball_dir = 0;
Bool prev_ball_see = false;
uint8_t prev_goal_dir = 0;
Bool prev_goal_see = false;
Bool prev_ball_have = false;

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
} menu_info =
  {
      { 2, 2, 2, 4 },
      { 1, 1, 1, 3 },
      { 1, 1, 1, 1 }
  };

Bool shutdown_confirmed = 0;

char sprintf_buf[21];

static void menu_main(event_t event1);
static void menu_match(event_t event1);
static void menu_sensors(event_t event1);
static void menu_camera(event_t event1);
static void menu_compass(event_t event1);
static void menu_compass_calibration(event_t event1);
static void menu_line(event_t event1);
static void menu_line_calibration(event_t event1);
static void menu_settings(event_t event1);
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
            menu_match(event1);
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
        case MENU_SHUTDOWN:
            menu_shutdown(event1);
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
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_SHUTDOWN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

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

    float robot_speed = speed_preset;
    float robot_dir = 0;
    float robot_trn = 0;

    estimate_rel_deviation();

    if(!stm.line.see)
    {
        if(rtm.ball.have)
        {
            if(rtm.goal.see)
            {
                if(abs(rtm.goal.dir) <= rtm.goal.diff)
                {
                    robot_speed = 30;
                }
                else
                {
                    robot_speed = 10;
                }
                robot_trn += rtm.goal.dir;
            }
            else
            {
                if(abs(compass_dev) < 10)
                {
                    robot_speed = 30;
                }
                else
                {
                    robot_speed = 10;
                }
                robot_trn += compass_dev;
            }
        }
        else
        {
            if(rtm.ball.see)
            {
                if(rtm.goal.see)
                {
                    if(abs(rtm.ball.dir) < 3 && abs(rtm.goal.dir) <= rtm.goal.diff)
                    {
                        robot_speed = 30;
                    }
                    else
                    {
                        if(rtm.ball.dir > rtm.goal.dir)
                        {
                            robot_dir = 90;
                        }
                        else
                        {
                            robot_dir = -90;
                        }
                        
                        if(abs(rtm.goal.dir) <= rtm.goal.diff)
                        {
                            robot_trn += rtm.goal.dir;
                        }
                        else
                        {
                            robot_trn += rtm.ball.dir;
                        }
                        robot_speed = 10;
                    }
                }
                else
                {
                    if(abs(rtm.ball.dir) < 3 && abs(compass_dev) < 10)
                    {
                        robot_speed = 30;
                        robot_dir = rtm.ball.dir;
                        robot_trn += compass_dev;
                    }
                    else
                    {
                        if(compass_dev > 0)
                        {
                            robot_dir = 90;
                        }
                        else
                        {
                            robot_dir = -90;
                        }
                        robot_trn += rtm.ball.dir;
                    }
                }
            }
            else
            {
                robot_speed = 10;
                robot_dir = 180;
                robot_trn += compass_dev;
            }
        }
    }
    else
    {
        robot_dir = stm.line.esc;
        robot_speed = 30;
    }

    robot_dir *= (3.14159265359f / 180.0f);
    
    mleft = robot_speed * (cos(robot_dir) * CosinMA1 - sin(robot_dir) * SinMA1);
    mright = robot_speed * (cos(robot_dir) * CosinMA2 - sin(robot_dir) * SinMA2);
    mrear = robot_speed * (cos(robot_dir) * CosinMA3 - sin(robot_dir) * SinMA3);
    
    mleft += robot_trn;
    mright += robot_trn;
    mrear += robot_trn;
    
    compensate_motor_output(mleft, mright, mrear);

    update_motor(mleft, mright, mrear);

    switch (event1)
    {
        case EVENT_BUTTON_RETURN_P:
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
    if(print_menu)
    {
        lcd_clear();
    }
    
    if(rtm.ball.dir != prev_ball_dir || rtm.ball.see != prev_ball_see || print_menu)
    {
        if (rtm.ball.dir == 0)
        {
            lcd_print_s(1, 0, "RPi inactive ");
        }
        else if (rtm.ball.see) //  && rtm.ball.dir != 0
        {
            sprintf(sprintf_buf, "Ball: %4d   ", rtm.ball.dir - 32);
            lcd_print_s(1, 0, sprintf_buf);
        }
        else
        {
            lcd_print_s(1, 0, "no ball found");
        }
        prev_ball_dir = rtm.ball.dir;
        prev_ball_see = rtm.ball.see;
    }

    if(rtm.goal.dir != prev_goal_dir || rtm.goal.see != prev_goal_see || print_menu)
    {
        if (rtm.goal.dir == 0)
        {
            lcd_print_s(2, 0, "RPi inactive ");
        }
        else if (rtm.goal.see)
        {
            sprintf(sprintf_buf, "Goal: %4d   ", rtm.goal.dir - 32);
            lcd_print_s(2, 0, sprintf_buf);
        }
        else
        {
            lcd_print_s(2, 0, "no goal found");
        }
        prev_goal_dir = rtm.goal.dir;
        prev_goal_see = rtm.goal.see;
    }
    
    if(rtm.ball.have != prev_ball_have || print_menu)
    {
        sprintf(sprintf_buf, "Having ball: %1d", rtm.ball.have);
        lcd_print_s(3, 0, sprintf_buf);
        prev_ball_have = rtm.ball.have;
    }
    
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
        sprintf(sprintf_buf, "Esc: %4d", stm.line.esc - 180);
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
                mts.line_cal_value -= 1;
                print_menu = true;
            }
            break;
        case EVENT_BUTTON_RIGHT_P:
            if(mts.line_cal_value < 16)
            {
                mts.line_cal_value += 1;
                print_menu = true;
            }
            break;
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_LINE;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_settings(event_t event1)
{
    FIL noOS_ini_file;

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
                        if (f_open(&noOS_ini_file, "noOS.ini", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
                        {
                            iniparser_set(noOS_ini_dict, "general:robot_id", (char*)&robot_id);
                            iniparser_dump_ini(noOS_ini_dict, &noOS_ini_file);
                            f_close(&noOS_ini_file);
                        }
                        else
                        {
                            // todo error menu
                        }
                        print_menu = true;
                    }
                    break;
                case 2:
                    
                    break;
                case 3:
                    
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
                        if (f_open(&noOS_ini_file, "noOS.ini", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
                        {
                            iniparser_set(noOS_ini_dict, "general:robot_id", (char*)&robot_id);
                            iniparser_dump_ini(noOS_ini_dict, &noOS_ini_file);
                            f_close(&noOS_ini_file);
                        }
                        else
                        {
                            // todo error menu
                        }
                        print_menu = true;
                    }
                    break;
                case 2:
                    
                    break;
                case 3:
                    
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
            act_menu = MENU_MAIN;
            print_menu = true;
            break;
        default:
            print_menu = false;
            break;
    }
}

static void menu_shutdown(event_t event1)
{
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
        while (ioport_get_pin_level(RPI2) == 1);
        mdelay(7500);
        
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
            //check_bat();
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
    const char* text[4] = {" Ball", " Compass", " Line", " "};
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
    sprintf(sprintf_buf, " Robot id: %1d", robot_id);
    text[0] = sprintf_buf;
//    lcd_print_m(text);
    lcd_clear();
    lcd_print_s(1, 0, text[0]);
    lcd_print_s(2, 0, text[1]);
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
