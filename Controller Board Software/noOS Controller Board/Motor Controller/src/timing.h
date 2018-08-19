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
uint32_t ticks_heartbeat;
Bool heart_state;

uint32_t getTicks(void);
void mdelay(uint32_t ul_dly_ticks);
void update_comm(void);
void update_heartbeat(void);

#endif