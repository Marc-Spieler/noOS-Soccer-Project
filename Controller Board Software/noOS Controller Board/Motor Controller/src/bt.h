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
            uint8_t setLed              :1;
            uint8_t setLineCalibration  :1;
            uint8_t startAction         :1;
            uint8_t rsvd                :4;
            uint8_t sbit                :1;
        } sbyte;
        uint8_t full_sbyte;
    };
        
    uint8_t newLineCalibration;
} bt_rx_t;

typedef struct
{
    union
    {
        struct
        {
            uint8_t rsvd	:7;
            uint8_t sbit	:1;
        } sbyte;
        uint8_t full_sbyte;
    };
        
    uint8_t battery_percentage;
        
    struct
    {
        uint8_t ls1     :1;
        uint8_t ls2 	:1;
        uint8_t ls3     :1;
        uint8_t ls4     :1;
        uint8_t ls5     :1;
        uint8_t ls6     :1;
        uint8_t ls7     :1;
        uint8_t blocked :1;
    } line_part_1;
    struct
    {
        uint8_t ls8     :1;
        uint8_t ls9     :1;
        uint8_t ls10    :1;
        uint8_t ls11    :1;
        uint8_t ls12    :1;
        uint8_t rsvd    :2;
        uint8_t blocked :1;
    } line_part_2;
    uint8_t currentLineCalibration;
        
    struct
    {
        uint8_t bit0_6  :7;
        uint8_t blocked :1;
    } absoluteCompassPart1;
    struct
    {
        uint8_t bit7_11 :5;
        uint8_t rsvd    :2;
        uint8_t blocked :1;
    } absoluteCompassPart2;
        
    struct
    {
        uint8_t bit0_6  :7;
        uint8_t blocked :1;
    } relativeCompassPart1;
    struct
    {
        uint8_t bit7_11 :5;
        uint8_t rsvd    :2;
        uint8_t blocked :1;
    } relativeCompassPart2;
} bt_tx_t;

extern bt_rx_t bt_rx;
extern bt_tx_t bt_tx;

void bt_init(void);
void serial_write(uint8_t *pbuf, uint8_t len);
void bt_maintenance(void);

#endif
