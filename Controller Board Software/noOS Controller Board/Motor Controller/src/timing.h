/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.07.18                                                    */
/************************************************************************/

#ifndef TIMING_H
#define TIMING_H

#include "asf.h"

uint32_t ticks_button_update;
uint32_t ticks_comm;
extern uint32_t ul_ticks_compass;

uint32_t getTicks(void);
void mdelay(uint32_t ul_dly_ticks);
void update_comm(void);
void update_battery(Bool update_forced);
void update_heartbeat(void);
void check_battery(void);
//void init_battery_warning(void);

#endif