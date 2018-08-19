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
	MENU_SETTINGS
} menu_t;

extern menu_t act_menu;
extern Bool print_menu;

extern int8_t rbt_id;

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

extern uint8_t act_cursor_line_on_lcd;
extern uint8_t prev_cursor_line_on_lcd;
extern uint8_t act_cursor_line;
extern uint8_t prev_cursor_line;
extern uint8_t min_cursor_line;
extern uint8_t max_cursor_line;
extern uint8_t menu_main_column;
extern uint8_t menu_main_scroll;

extern uint32_t cnt;

void menu(event_t event1);
void menu_main(event_t event1);
void menu_settings(event_t event1);
void print_menu_main(void);
void print_cursor(void);
event_t button_events(void);

#endif