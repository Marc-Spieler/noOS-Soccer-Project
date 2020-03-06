/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.07.2019                                                  */
/************************************************************************/

#ifndef BT_H
#define BT_H

#include "asf.h"

extern uint32_t bt_rx_ticks;

typedef struct
{
    union
    {
        struct
        {
            uint8_t sbit	 :1;
            uint8_t rsvd	 :4;
            uint8_t active	 :1;
			uint8_t ball_see :1;
            uint8_t at_goal	 :1;
        } sbyte;
        uint8_t full_sbyte;
    };
    
    int8_t ball_angle;
    int8_t goal_angle;
    uint8_t ball_dist;
    uint8_t goal_dist;
} bt_rx_t;

typedef struct
{
    union
    {
        struct
        {
            uint8_t at_goal	 :1;
            uint8_t ball_see :1;
			uint8_t active	 :1;
            uint8_t rsvd	 :4;
            uint8_t sbit	 :1;
        } sbyte;
        uint8_t full_sbyte;
    };
    
    uint8_t ball_angle;
    uint8_t goal_angle;
    uint8_t ball_dist;
    uint8_t goal_dist;
} bt_tx_t;

extern bt_rx_t bt_rx;
extern bt_tx_t bt_tx;

void bt_init(void);
void bt_write(uint8_t *pbuf, uint8_t len);
void bt_maintenance(void);

#endif
