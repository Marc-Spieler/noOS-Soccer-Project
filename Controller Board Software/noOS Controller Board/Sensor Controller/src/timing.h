/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 22.08.18                                                    */
/************************************************************************/

#ifndef TIMING_H
#define TIMING_H

#include "asf.h"

uint32_t getTicks(void);
void mdelay(uint32_t ul_dly_ticks);
void update_battery(void);

#endif
