/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 03.03.19                                                    */
/************************************************************************/

#include "compass.h"
#include "lcd.h"
#include "timing.h"
#include "comm.h"

uint16_t direction;
int16_t opponent_goal;

static uint8_t compassIsBusy = false;

static void compass_callback(void);

void compass_init(void)
{
    twi_packet_t *rx_packet = twi_get_rx_packet();

    rx_packet->chip = 0x60;
    rx_packet->addr[0] = 0x02;
    rx_packet->addr_length = 1;
    rx_packet->length = sizeof(direction);

    twi_set_compass_tx_callback(compass_callback);
    twi_set_compass_rx_callback(compass_callback);
}

void update_compass(void)
{
    twi_packet_t *rx_packet = twi_get_rx_packet();

    if ((getTicks() - ul_ticks_compass) > 100)
    {
        if(lcd_is_busy())
        {
            return;
        }

        ul_ticks_compass = getTicks();
        
        compassIsBusy = true;
        if(twi_pdc_master_read(TWI0, rx_packet) == TWI_SUCCESS)
        {
            while(compassIsBusy);
        }
        direction = (rx_packet->buffer[0] << 8) | rx_packet->buffer[1];
    }
}

void set_compass_is_busy(void)
{
    compassIsBusy = true;
}

uint8_t compass_is_busy(void)
{
    return compassIsBusy;
}

void set_opponent_goal(void)
{
    opponent_goal = direction;
}

void set_inverted_opponent_goal(void)
{
    update_compass();
    opponent_goal = (direction - 1800);
}

void estimate_rel_deviation(void)
{
    update_compass();
    float rel_dev = (float)(direction - opponent_goal) / 10;

    while(rel_dev >= 180.0f) rel_dev -= 360.0f;
    while(rel_dev <= -180.0f) rel_dev += 360.0f;
    
    s.compass = rel_dev;
}

/*float update_correction(pidReg_t *reg)
{
    twi_packet_t *rx_packet = twi_get_rx_packet();
    int16_t tmp_dev;
    static float tmp_corr = 0.0f; // todo eventuell einen sinnvollen Fehlerwert zur?ckgeben
    
    if(lcd_is_busy())
    {
        return tmp_corr;
    }

    if ((getTicks() - ul_ticks_compass) > COMPASS_UPDATE_RATE)
    {
        ul_ticks_compass = getTicks();
        
        compassIsBusy = true;
        if(twi_pdc_master_read(TWI0, rx_packet) == TWI_SUCCESS)
        {
            while(compassIsBusy);
        }
        direction = (rx_packet->buffer[0] << 8) | rx_packet->buffer[1];
        tmp_dev = estimate_rel_deviation(direction, opponent_goal);
        tmp_corr = -pidReg(reg, 0.0f, (float)tmp_dev);
    }
    
    return tmp_corr;
}*/

//local function
static void compass_callback(void)
{
    compassIsBusy = false;
}