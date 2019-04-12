/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 19.08.18                                                    */
/************************************************************************/

#ifndef MENU_H
#define MENU_H

#include "asf.h"

typedef enum
{
    EVENT_NO_EVENT,
    EVENT_BUTTON_UP_P,
    EVENT_BUTTON_UP_H,
    EVENT_BUTTON_UP_R,
    EVENT_BUTTON_LEFT_P,
    EVENT_BUTTON_LEFT_H,
    EVENT_BUTTON_LEFT_R,
    EVENT_BUTTON_MID_P,
    EVENT_BUTTON_MID_H,
    EVENT_BUTTON_MID_R,
    EVENT_BUTTON_RIGHT_P,
    EVENT_BUTTON_RIGHT_H,
    EVENT_BUTTON_RIGHT_R,
    EVENT_BUTTON_DOWN_P,
    EVENT_BUTTON_DOWN_H,
    EVENT_BUTTON_DOWN_R,
    EVENT_BUTTON_RETURN_P,
    EVENT_BUTTON_RETURN_H,
    EVENT_BUTTON_RETURN_R
} event_t;

typedef enum
{
    MENU_MAIN,
    MENU_MATCH,
    MENU_SENSORS,
    MENU_SETTINGS,
    MENU_CAMERA,
    MENU_LINE,
    MENU_LINE_CALIBRATION,
    MENU_COMPASS,
    MENU_COMPASS_CALIBRATION,
    MENU_SHUTDOWN
} menu_t;

extern menu_t act_menu;
extern Bool print_menu;

extern uint8_t rbt_id;
extern uint8_t speed_preset;
extern Bool allow_leds;

char sprintf_cache[20];

Bool pb_up_act;
Bool pb_up_prev;
Bool pb_left_act;
Bool pb_left_prev;
Bool pb_mid_act;
Bool pb_mid_prev;
Bool pb_right_act;
Bool pb_right_prev;
Bool pb_down_act;
Bool pb_down_prev;
Bool pb_return_act;
Bool pb_return_prev;

void menu(event_t event1);
event_t button_events(void);

#endif
