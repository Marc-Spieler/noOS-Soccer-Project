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
    
    uint16_t rsvd;
} motor_to_sensor_t;

typedef struct
{
    struct
    {
        uint8_t heartbeat	 :1;
        uint8_t rsvd		 :7;
    } ibit;
    
    uint8_t bat_voltage;
    uint8_t bat_percentage;
} sensor_to_motor_t;

extern motor_to_sensor_t mts;
extern sensor_to_motor_t stm;

typedef struct
{
    
} motor_to_raspberrypi_t;

typedef struct
{
    
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
