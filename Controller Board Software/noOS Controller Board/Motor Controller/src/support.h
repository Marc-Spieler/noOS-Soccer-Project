/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 24.04.2019                                                  */
/************************************************************************/

#ifndef SUPPORT_H
#define SUPPORT_H

#include "asf.h"
#include "iniparser.h"

extern dictionary* noOS_ini_dict;

extern uint16_t robot_id;
extern uint16_t speed_preset;
extern Bool heartbeat;
extern Bool allow_leds;

void set_led(ioport_pin_t pin, Bool level);
void create_default_ini_file(void);
void parse_ini_file(void);

#endif