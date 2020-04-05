/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.04.2020                                                  */
/************************************************************************/

#include "compass.h"
#include "string.h"
#include "data.h"
#include "timing.h"

twi_master_options_t twiConfig;
uint32_t lastCompassRead = 0;
static uint8_t compassIsBusy = false;

uint16_t rawCompass = 0;
float goalReferencePoint = 0.0f;

// local functions
static void compassCallback(void);

void compassInit(void)
{
    memset((void *)&twiConfig, 0, sizeof(twiConfig));
    twiConfig.speed = 100000;
    twi_master_setup(TWI0, &twiConfig);
    
    twi_packet_t *rx_packet = twi_get_rx_packet();

    rx_packet->chip = 0x60;
    rx_packet->addr[0] = 0x02;
    rx_packet->addr_length = 1;
    rx_packet->length = 2;
    
    twi_set_compass_rx_callback(compassCallback);
    twi_set_compass_tx_callback(compassCallback);
}

void compassMaintenance(void)
{
    twi_packet_t *rx_packet = twi_get_rx_packet();
    
    if((getTicks() - lastCompassRead) >= 100)
    {
        lastCompassRead = getTicks();
        
        if(twi_pdc_master_read(TWI0, rx_packet) == TWI_SUCCESS)
        {
            compassIsBusy = true;
            while(compassIsBusy);
        }
        rawCompass = ((rx_packet->buffer[0] << 8) | rx_packet->buffer[1]);
        
        if((int16_t)goalReferencePoint == 0) setGoalReference(false);
        
        float goalDeviation = ((float)(rawCompass - goalReferencePoint) / 10.0f);

        while(goalDeviation > 180.0f) goalDeviation -= 360.0f;
        while(goalDeviation <= -180.0f) goalDeviation += 360.0f;
        
        data.compass = goalDeviation;
    }
}

void setGoalReference(Bool inverted)
{
    if(inverted) goalReferencePoint = (rawCompass - 1800);
    else goalReferencePoint = rawCompass;
}

void compassCalibrationStep(void)
{
    twi_packet_t *tx_packet = twi_get_tx_packet();
    
    tx_packet->chip = 0x60;
    tx_packet->addr[0] = 0x0f;
    tx_packet->addr_length = 1;
    
    tx_packet->buffer[0] = 0xff;
    tx_packet->length = 1;
    
    twi_pdc_master_write(TWI0, tx_packet);
}

static void compassCallback(void)
{
    compassIsBusy = false;
}