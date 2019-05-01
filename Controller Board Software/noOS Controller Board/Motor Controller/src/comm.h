/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 19.08.18                                                    */
/************************************************************************/

#ifndef COMM_H
#define COMM_H

#include "asf.h"

typedef struct
{
    struct
    {
        uint8_t heartbeat	 :1;
        uint8_t button  	 :1;
        uint8_t sleep_mode   :1;
        uint8_t rsvd		 :5;
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
            } single;
            uint16_t all;
        };
        
        uint32_t see         :1;
        uint32_t esc         :9;
        uint32_t rsvd        :10;
    } line;
    
    struct
    {
        uint16_t voltage     :8;
        uint16_t percentage  :8;
    } battery;
} sensor_to_motor_t;

extern motor_to_sensor_t mts;
extern sensor_to_motor_t stm;

typedef struct
{
    uint16_t rsvd;
} motor_to_raspberrypi_t;

typedef struct
{
    struct
    {
        uint8_t dir         :6;
        uint8_t see         :1;
        uint8_t have        :1;
    } ball;
    
    struct
    {
        uint16_t dir         :6;
        uint16_t see         :1;
        uint16_t diff        :5;
        uint16_t rsvd        :4;
    } goal;

    uint8_t rsvd;
} raspberrypi_to_motor_t;

extern motor_to_raspberrypi_t mtr;
extern raspberrypi_to_motor_t rtm;

extern uint8_t sens_buf[sizeof(stm)];
extern uint8_t rpi_buf[sizeof(rtm)];

//static Bool b_trigger = false;

void spi_init(void);
void spi_master_initialize(void);
void spi_slave_initialize(void);
void spi_master_transfer(void *p_buf, uint32_t ul_size);
void spi_slave_transfer(void *p_buf, uint32_t ul_size);
void configure_dmac(void);
void PrepareValuesToSend(void);

#endif
