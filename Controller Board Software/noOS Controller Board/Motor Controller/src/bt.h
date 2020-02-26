/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.07.2019                                                  */
/************************************************************************/

#ifndef BT_H
#define BT_H

#include "asf.h"

// Table with all events used in eEMU application
/*#define BT_EVENT_CALL               "CALL"
#define BT_EVENT_CONNECT            "CONNECT"
#define BT_EVENT_INQUIRY            "INQUIRY"
#define BT_EVENT_INQUIRY_PARTIAL    "INQUIRY_PARTIAL"
#define BT_EVENT_NO_CARRIER         "NO CARRIER"
#define BT_EVENT_OK                 "OK"
#define BT_EVENT_PAIR               "PAIR"
#define BT_EVENT_READY              "READY."
#define BT_EVENT_SSP_COMPLETE       "SSP COMPLETE"
#define BT_EVENT_SSP_PASSKEY        "SSP PASSKEY"
#define BT_EVENT_SYNTAX_ERROR       "SYNTAX ERROR"

#define BT_EVENT_TABLE              \
{                                   \
    BT_EVENT_CALL,                  \
    BT_EVENT_CONNECT,               \
    BT_EVENT_INQUIRY,               \
    BT_EVENT_INQUIRY_PARTIAL,       \
    BT_EVENT_NO_CARRIER,            \
    BT_EVENT_OK,                    \
    BT_EVENT_PAIR,                  \
    BT_EVENT_READY,                 \
    BT_EVENT_SSP_COMPLETE,          \
    BT_EVENT_SSP_PASSKEY,           \
    BT_EVENT_SYNTAX_ERROR           \
}*/
/*
 * Index to events in table
 */
/*typedef enum
{
    BT_EVENT_CALL_ID = 0,
    BT_EVENT_CONNECT_ID,
    BT_EVENT_INQUIRY_ID,
    BT_EVENT_INQUIRY_PARTIAL_ID,
    BT_EVENT_NO_CARRIER_ID,
    BT_EVENT_OK_ID,
    BT_EVENT_PAIR_ID,
    BT_EVENT_READY_ID,
    BT_EVENT_SSP_COMPLETE_ID,
    BT_EVENT_SSP_PASSKEY_ID,
    BT_EVENT_SYNTAX_ERROR_ID,
    BT_EVENT_COUNT
} bt_event_id_t;

typedef enum
{
    //BT_RES_OK,
    BT_RES_MATCH,
    BT_RES_PID_TUNER
} bt_response_t;

extern bt_response_t expected_bt_response;*/

extern uint32_t bt_rx_ticks;

typedef struct
{
    union
    {
        struct
        {
            uint8_t sbit	 :1;
            uint8_t rsvd	 :5;
            uint8_t active	 :1;
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
            uint8_t active	 :1;
            uint8_t rsvd	 :5;
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

#endif
