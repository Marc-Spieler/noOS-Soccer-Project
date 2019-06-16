/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 03.03.19                                                    */
/************************************************************************/

#ifndef COMPASS_H
#define COMPASS_H

#include "asf.h"

void compass_init(void);
void update_compass(void);
void set_compass_is_busy(void);
uint8_t compass_is_busy(void);
void set_opponent_goal(void);
void set_inverted_opponent_goal(void);
void estimate_rel_deviation(void);
//float update_correction(pidReg_t *reg);

#endif
