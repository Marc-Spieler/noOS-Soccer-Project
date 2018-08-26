/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 11.02.18                                                    */
/************************************************************************/

#ifndef SUPPORT_H
#define SUPPORT_H

#include "asf.h"
#include "lcd.h"
#include "comm.h"
#include "timing.h"
#include "pid_reg.h"
#include "string.h"
#include "math.h"

extern uint8_t act_cursor_line;
extern uint8_t prev_cursor_line;
extern uint8_t max_cursor_line;

extern Bool prev_pbTop_state;
extern Bool prev_pbLeft_state;
extern Bool prev_pbMid_state;
extern Bool prev_pbRight_state;
extern Bool prev_pbBot_state;
extern Bool prev_pbReturn_state;

extern uint16_t direction;

void reset_prev_states(void);
void update_prev_states(void);
void update_cursor_line(uint8_t max_line);
void set_cursor(uint8_t line);
void update_comm(void);
void compass_init(void);
void update_compass(void);
float update_correction(pidReg_t *reg);
void update_bat(void);
void check_bat(void);
float my_atan2(int16_t x, int16_t y);

#endif
