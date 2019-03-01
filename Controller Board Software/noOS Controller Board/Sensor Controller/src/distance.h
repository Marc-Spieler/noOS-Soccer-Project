/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 30.11.18                                                    */
/************************************************************************/

#ifndef DISTANCE_H
#define DISTANCE_H

#include "asf.h"

typedef struct
{
    uint8_t front;
    uint8_t left;
    uint8_t right;
    uint8_t rear;
} dsense_t;

void init_distance(void);
void update_distance(void);

#endif
