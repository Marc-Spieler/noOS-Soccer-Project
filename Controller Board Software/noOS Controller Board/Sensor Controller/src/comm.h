/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 21.08.18                                                    */
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

void spi_init(void);
void spi_slave_transfer(void *p_buf, uint32_t ul_size);
void spi_slave_initialize(void);
void configure_dmac(void);
void PrepareValuesToSend(void);

#endif
