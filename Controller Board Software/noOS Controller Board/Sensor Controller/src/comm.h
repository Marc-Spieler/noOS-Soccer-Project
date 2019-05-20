/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 21.08.2018                                                  */
/************************************************************************/

#ifndef COMM_H
#define COMM_H

#include "asf.h"

typedef struct
{
    struct
    {
        uint8_t heartbeat	 :1;
        uint8_t sleep_mode   :1;
        uint8_t rsvd		 :6;
    } ibit;
    
    uint8_t line_cal_value;
    uint8_t rsvd_1;
    uint32_t rsvd_2;
} motor_to_sensor_t;

typedef struct
{
    struct
    {
        uint8_t heartbeat	 :1;
        uint8_t rsvd		 :7;
    } ibit;
    
    struct
    {
        union
        {
            struct
            {
                uint16_t segment_1   :1;
                uint16_t segment_2   :1;
                uint16_t segment_3   :1;
                uint16_t segment_4   :1;
                uint16_t segment_5   :1;
                uint16_t segment_6   :1;
                uint16_t segment_7   :1;
                uint16_t segment_8   :1;
                uint16_t segment_9   :1;
                uint16_t segment_10  :1;
                uint16_t segment_11  :1;
                uint16_t segment_12  :1;
                uint16_t rsvd        :4;
            } single;
            uint16_t all;
        };
        
        struct
        {
            uint16_t see         :1;
            uint16_t esc         :9;
            uint16_t rsvd        :6;
        };
    } line;
    
    struct
    {
        uint16_t voltage     :8;
        uint16_t percentage  :8;
    } battery;
} sensor_to_motor_t;

extern motor_to_sensor_t mts;
extern sensor_to_motor_t stm;

void spi_init(void);
void spi_slave_transfer(void *p_buf, uint32_t ul_size);
void spi_slave_initialize(void);
void configure_dmac(void);
void PrepareValuesToSend(void);

#endif
