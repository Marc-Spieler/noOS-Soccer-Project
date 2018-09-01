/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 19.08.18                                                    */
/************************************************************************/

#include "menu.h"
#include "lcd.h"
#include "timing.h"

menu_t act_menu = MENU_MAIN;
Bool print_menu = 1;

int8_t rbt_id = 1;

uint8_t act_cursor_line_on_lcd = 1;
uint8_t prev_cursor_line_on_lcd = 1;
uint8_t act_cursor_line = 1;
uint8_t prev_cursor_line = 1;
uint8_t min_cursor_line = 1;
uint8_t max_cursor_line = 8;
uint8_t menu_main_column = 1;
uint8_t menu_main_scroll = 0;

uint32_t cnt = 0;


void menu(event_t event1)
{
    switch (event1)
    {
        case EVENT_BUTTON_UP_P:
            if (act_cursor_line > min_cursor_line)
            {
                act_cursor_line--;
                if (act_cursor_line_on_lcd == 1)
                {
                    if (act_menu == MENU_MAIN)
                    {
                        menu_main_scroll--;
                        print_menu_main();
                    }
                }
                else
                {
                    act_cursor_line_on_lcd--;
                    print_cursor();
                }
            }
            break;
        case EVENT_BUTTON_DOWN_P:
            if (act_cursor_line < max_cursor_line)
            {
                act_cursor_line++;
                if (act_cursor_line_on_lcd == 4)
                {
                    if (act_menu == MENU_MAIN)
                    {
                        menu_main_scroll++;
                        print_menu_main();
                    }
                }
                else
                {
                    act_cursor_line_on_lcd++;
                    print_cursor();
                }
            }
            break;
        default:
            break;
    }
	
    switch (act_menu)
    {
        case MENU_MAIN:
            menu_main(event1);
            break;
        case MENU_SETTINGS:
            menu_settings(event1);
            break;
        default:
            break;
    }
}

void menu_main(event_t event1)
{
    switch (event1)
    {
        case EVENT_BUTTON_MID_P:
            switch (menu_main_column)
            {
                case 0:
                    switch (act_cursor_line)
                    {
                        case 2:
                            break;
                        case 3:
                            break;
                        case 4:
                            break;
                        case 5:
                    
                            break;
                        case 6:
                    
                            break;
                        case 7:
                            break;
                        case 8:
                            break;
                        default:
                            break;
                    }
                    break;
                case 1:
                    switch (act_cursor_line)
                    {
                        case 2:
                            //act_menu =
                            break;
                        case 3:
                            //act_menu =
                            break;
                        case 4:
                            //act_menu =
                            break;
                        case 5:
                            break;
                        case 6:
                            break;
                        case 7:
                            break;
                        case 8:
                            break;
                        default:
                            break;
                    }
                    break;
                case 2:
                    switch (act_cursor_line)
                    {
                        case 2:
                            //act_menu =
                            break;
                        case 3:
				        
                            break;
                        case 4:
				        
                            break;
                        case 5:
				        
                            break;
                        case 6:
				        
                            break;
                        case 7:
				        
                            break;
                        case 8:
                            break;
                        default:
                            break;
                    }
                    break;
            }
            break;
        case EVENT_BUTTON_LEFT_P:
            if (act_cursor_line == 1)
            {
                if (menu_main_column >= 1)
                {
                    menu_main_column--;
                }
                print_menu = 1;
            }
            break;
        case EVENT_BUTTON_RIGHT_P:
            if (act_cursor_line == 1)
            {
                if (menu_main_column <= 1)
                {
                    menu_main_column++;
                }
                print_menu = 1;
            }
            break;
        default:
            break;
    }
	
    if (print_menu)
    {
        print_menu = 0;
        print_menu_main();
    }
}

void menu_settings(event_t event1)
{
	
}

void print_menu_main(void)
{
    const char *menu_text[3][8] =
    {
        {"   Settings  ", " Robot ID:", " Speed:", " WIFI:", " Calibrate", " Set field size ref", "", ""},
        {"     Match  ", " Start match", " Sensor values", " Drive angle pid", "", "", "", ""},
        {"     Tests", " Turn to start", " Move to middle", " Move to ball", " RPI", " Stop on line", " Encoder test", ""}
    };
    char *menu_display[4];

    lcd_clear();

    menu_display[0] = (char *)menu_text[menu_main_column][menu_main_scroll + 0];
    menu_display[1] = (char *)menu_text[menu_main_column][menu_main_scroll + 1];
    menu_display[2] = (char *)menu_text[menu_main_column][menu_main_scroll + 2];
    menu_display[3] = (char *)menu_text[menu_main_column][menu_main_scroll + 3];
    lcd_print_m(menu_display);

    switch(menu_main_column)
    {
        case 0:
            sprintf(sprintf_cache, "%1d", rbt_id);
            lcd_print_s(2 - menu_main_scroll, 11, sprintf_cache);
            sprintf(sprintf_cache, "%2d", 15);	//speed_preset
            lcd_print_s(3 - menu_main_scroll, 8, sprintf_cache);
            sprintf(sprintf_cache, "%1d", 1);	//rpi_tx.info.wifi
            lcd_print_s(4 - menu_main_scroll, 7, sprintf_cache);
            break;
        case 1:
            break;
        case 2:
            break;
    }
    
    print_cursor();
    update_battery(1);
}

void print_cursor(void)
{
    if (act_menu == MENU_MAIN && act_cursor_line == 1)
    {
        switch (menu_main_column)
        {
            case 0:
                lcd_print_s(1, 12, ">");
                lcd_print_s(2, 0, " ");
                break;
            case 1:
                lcd_print_s(1, 3, "<");
                lcd_print_s(1, 11, ">");
                lcd_print_s(2, 0, " ");
                break;
            case 2:
                lcd_print_s(1, 3, "<");
                lcd_print_s(2, 0, " ");
                break;
            default:
                break;
        }
    }
    else
    {
        if (prev_cursor_line == 1)
        {
            switch (menu_main_column)
            {
                case 0:
                    lcd_print_s(1, 12, " ");
                    break;
                case 1:
                    lcd_print_s(1, 3, " ");
                    lcd_print_s(1, 11, " ");
                    break;
                case 2:
                    lcd_print_s(1, 3, " ");
                    break;
                default:
                    break;
            }
        }
        
        lcd_print_s(prev_cursor_line_on_lcd, 0, " ");
        lcd_print_s(act_cursor_line_on_lcd, 0, ">");
    }
    
    prev_cursor_line = act_cursor_line;
    prev_cursor_line_on_lcd = act_cursor_line_on_lcd;
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
        else if (pb_up_act == pb_up_prev && pb_up_act == 0)
        {
            nextEvent = EVENT_BUTTON_UP_H;
        }
        
        if (pb_left_act != pb_left_prev && pb_left_act == 0)
        {
            nextEvent = EVENT_BUTTON_LEFT_P;
        }
        else if (pb_left_act != pb_left_prev && pb_left_act == 1)
        {
            nextEvent = EVENT_BUTTON_LEFT_R;
        }
        else if (pb_left_act == pb_left_prev && pb_left_act == 0)
        {
            nextEvent = EVENT_BUTTON_LEFT_H;
        }
        
        if (pb_mid_act != pb_mid_prev && pb_mid_act == 0)
        {
            nextEvent = EVENT_BUTTON_MID_P;
        }
        else if (pb_mid_act != pb_mid_prev && pb_mid_act == 1)
        {
            nextEvent = EVENT_BUTTON_MID_R;
        }
        else if (pb_mid_act == pb_mid_prev && pb_mid_act == 0)
        {
            nextEvent = EVENT_BUTTON_MID_H;
        }
        
        if (pb_right_act != pb_right_prev && pb_right_act == 0)
        {
            nextEvent = EVENT_BUTTON_RIGHT_P;
        }
        else if (pb_right_act != pb_right_prev && pb_right_act == 1)
        {
            nextEvent = EVENT_BUTTON_RIGHT_R;
        }
        else if (pb_right_act == pb_right_prev && pb_right_act == 0)
        {
            nextEvent = EVENT_BUTTON_RIGHT_H;
        }
        
        if (pb_down_act != pb_down_prev && pb_down_act == 0)
        {
            nextEvent = EVENT_BUTTON_DOWN_P;
        }
        else if (pb_down_act != pb_down_prev && pb_down_act == 1)
        {
            nextEvent = EVENT_BUTTON_DOWN_R;
        }
        else if (pb_down_act == pb_down_prev && pb_down_act == 0)
        {
            nextEvent = EVENT_BUTTON_DOWN_H;
        }
        
        if (pb_return_act != pb_return_prev && pb_return_act == 0)
        {
            nextEvent = EVENT_BUTTON_RETURN_P;
        }
        else if (pb_return_act != pb_return_prev && pb_return_act == 1)
        {
            nextEvent = EVENT_BUTTON_RETURN_R;
        }
        else if (pb_return_act == pb_up_prev && pb_return_act == 0)
        {
            nextEvent = EVENT_BUTTON_RETURN_H;
        }
        
        pb_up_prev = pb_up_act;
        pb_left_prev = pb_left_act;
        pb_mid_prev = pb_mid_act;
        pb_right_prev = pb_right_act;
        pb_down_prev = pb_down_act;
        pb_return_prev = pb_return_act;
    }

    return nextEvent;
}
