/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 19.08.18                                                    */
/************************************************************************/

#include "menu.h"
#include "lcd.h"
#include "timing.h"
#include "compass.h"
#include "comm.h"

menu_t act_menu = MENU_MAIN;
Bool print_menu = 1;

uint8_t rbt_id = 1;
uint8_t speed_preset = 15;
Bool allow_leds = true;

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

uint8_t compass_cal_step = 0;
Bool shutdown_confirmed = 0;

char sprintf_buf[21];

static void menu_main(event_t event1);
static void menu_match(event_t event1);
static void menu_sensors(event_t event1);
static void menu_ball(event_t event1);
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
        case MENU_BALL:
            menu_ball(event1);
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
        print_menu = 0;
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
                        print_menu = 1;
                        break;
                    case 3:
                        act_menu = MENU_SENSORS;
                        print_menu = 1;
                        break;
                    case 4:
                        act_menu = MENU_SETTINGS;
                        print_menu = 1;
                        break;
                    default:
                        break;
                }
            }
            break;
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_SHUTDOWN;
            print_menu = 1;
            break;
        default:
            break;
    }
}

static void menu_match(event_t event1)
{
    
    
    if(event1 == EVENT_BUTTON_RETURN_P)
    {
        act_menu = MENU_MAIN;
        print_menu = 1;
    }
}

static void menu_sensors(event_t event1)
{
    if (print_menu)
    {
        print_menu = 0;
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
                    act_menu = MENU_BALL;
                    print_menu = 1;
                    break;
                case 2:
                    act_menu = MENU_COMPASS;
                    print_menu = 1;
                    break;
                case 3:
                    act_menu = MENU_LINE;
                    print_menu = 1;
                    break;
                default:
                    break;
            }
            break;
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_MAIN;
            print_menu = 1;
            break;
        default:
            break;
    }
}

static void menu_ball(event_t event1)
{
    if(print_menu)
    {
        print_menu = 0;
        lcd_clear();
    }
    
    /*if (rpi_rx.ibit.ball >= 100 && rpi_rx.ibit.ball <= 162)
    {
        sprintf(sprintf_buf, "Direction: %4d   ", rpi_rx.ibit.ball - 131);
        lcd_print_s(2, 0, sprintf_buf);
    }
    else if (rpi_rx.ibit.ball == 0)
    {
        lcd_print_s(2, 0, "Direction: waiting");
    }
    else
    {
        lcd_print_s(2, 0, "Direction: no ball");
    }
    
    sprintf(sprintf_buf, "Having ball: %1d", rpi_rx.ibit.have_ball);*/
    lcd_print_s(3, 0, sprintf_buf);
    
    if(event1 == EVENT_BUTTON_RETURN_P)
    {
        act_menu = MENU_SENSORS;
        print_menu = 1;
    }
}

static void menu_compass(event_t event1)
{
    static uint16_t prev_direction = 0;
    
    if(print_menu)
    {
        print_menu = 0;
        lcd_clear();
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
            print_menu = 1;
            break;
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_SENSORS;
            print_menu = 1;
            break;
        default:
            break;
    }
}

static void menu_compass_calibration(event_t event1)
{
    if(print_menu)
    {
        print_menu = 0;
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
            print_menu = 1;
        }
        print_menu = 1;
    }
}

static void menu_line(event_t event1)
{
    static uint16_t prev_line_values;

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
    }
    
    if(event1 == EVENT_BUTTON_RETURN_P)
    {
        act_menu = MENU_SENSORS;
        print_menu = 1;
    }
}

static void menu_line_calibration(event_t event1)
{
    
    
    if(event1 == EVENT_BUTTON_RETURN_P)
    {
        act_menu = MENU_LINE;
        print_menu = 1;
    }
}

static void menu_settings(event_t event1)
{
    if (print_menu)
    {
        print_menu = 0;
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
        case EVENT_BUTTON_MID_P:
            switch (menu_info.settings.act_cursor_line)
            {
                case 1:
                    act_menu = MENU_BALL;
                    print_menu = 1;
                    break;
                case 2:
                    act_menu = MENU_COMPASS;
                    print_menu = 1;
                    break;
                case 3:
                    act_menu = MENU_LINE;
                    print_menu = 1;
                    break;
                default:
                    break;
            }
            break;
        case EVENT_BUTTON_RETURN_P:
            act_menu = MENU_MAIN;
            print_menu = 1;
            break;
        default:
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
        
        /*pwm_channel_disable(PWM, DRIBBLER);
        pwm_channel_disable(PWM, MLEFT);
        pwm_channel_disable(PWM, MRIGHT);
        pwm_channel_disable(PWM, MREAR);
        
        pwm_channel_disable(PWM, ENC_CLK);
        
        sensor_parameters.ibit.sleep_mode = 1;*/
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
            print_menu = 0;
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
            print_menu = 1;
            break;
        default:
            break;
    }
}

static void print_menu_main(void)
{
    const char *text[4] = {"    noOS ONE", " Match", " Sensors", " Settings"};
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
    const char *text[4] = {" Ball", " Compass", " Line", " "};
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
    const char *text[4] = {" ", " ", " ", " "};
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
