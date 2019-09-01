/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.07.2019                                                  */
/************************************************************************/

#ifndef BT_H
#define BT_H

#include "asf.h"

// Table with all events used in eEMU application
#define BT_EVENT_CALL               "CALL"
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
}
/*
 * Index to events in table
 */
typedef enum
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

void bt_init(void);
void bt_write_string(uint8_t *pbuf, uint8_t len);
//uint8_t bt_read_byte(uint8_t *pbuf);

#endif
