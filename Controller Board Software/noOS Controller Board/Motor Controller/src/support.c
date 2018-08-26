/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 11.02.18                                                    */
/************************************************************************/
#if 0
#include "support.h"
#include "motor.h"
#include "lcd.h"

uint8_t act_cursor_line = 1;
uint8_t prev_cursor_line = 1;
uint8_t max_cursor_line;

Bool prev_pbTop_state = 1;
Bool prev_pbLeft_state = 1;
Bool prev_pbMid_state = 1;
Bool prev_pbRight_state = 1;
Bool prev_pbBot_state = 1;
Bool prev_pbReturn_state = 1;

uint16_t direction;

/************************************************************
* Local Variables                                           *
************************************************************/
static uint8_t compassIsBusy = false;

/************************************************************
* Local Function Prototypes                                 *
************************************************************/
static void compass_callback(void);

/************************************************************
* Functions                                                 *
************************************************************/
void reset_prev_states(void)
{
    prev_pbTop_state = 1;
    prev_pbLeft_state = 1;
    prev_pbMid_state = 1;
    prev_pbRight_state = 1;
    prev_pbBot_state = 1;
    prev_pbReturn_state = 1;
}

void update_prev_states(void)
{
    prev_pbTop_state = ioport_get_pin_level(PB_TOP);
    prev_pbLeft_state = ioport_get_pin_level(PB_LEFT);
    prev_pbMid_state = ioport_get_pin_level(PB_MID);
    prev_pbRight_state = ioport_get_pin_level(PB_RIGHT);
    prev_pbBot_state = ioport_get_pin_level(PB_BOT);
    prev_pbReturn_state = ioport_get_pin_level(PB_RETURN);
}

void update_cursor_line(uint8_t max_line)
{
    if(prev_pbBot_state == 1 && ioport_get_pin_level(PB_BOT) == 0)
    {
        if(act_cursor_line < max_line)
        {
            act_cursor_line++;
        }
    }

    if(prev_pbTop_state == 1 && ioport_get_pin_level(PB_TOP) == 0)
    {
        if(act_cursor_line > 1)
        {
            act_cursor_line--;
        }
    }
}

void set_cursor(uint8_t line)
{
    if (line > 4)
    {
        switch (line)
        {
            case 5:
            line = 1;
            break;
            case 6:
            line = 2;
            break;
            case 7:
            line = 3;
            break;
            case 8:
            line = 4;
            break;
        }
    }
    
    lcd_print_s(prev_cursor_line, 0, " ");
    lcd_print_s(line, 0, ">");
    prev_cursor_line = line;
}

void update_comm(void)
{
    if ((getTicks() - ul_ticks_comm) > COMM_UPDATE_RATE)
    {
        ul_ticks_comm = getTicks();
        
        memcpy(&g_ub_m_buf, &sensor_parameters, sizeof(sensor_parameters));
        spi_master_transfer(&g_ub_m_buf, sizeof(g_ub_m_buf));
    }
    
    PrepareValuesToSend();
}

void compass_init(void)
{
    twi_packet_t *rx_packet = twi_get_rx_packet();

    rx_packet->chip = 0x60;
    rx_packet->addr[0] = 0x02;
    rx_packet->addr_length = 1;
    rx_packet->length = sizeof(direction);

    twi_set_rx_callback(compass_callback);

    memset((void *)&twiConfig, 0, sizeof(twiConfig));
    twiConfig.speed = 100000;
    twi_master_setup(TWI0, &twiConfig);
}

void update_compass(void)
{
    twi_packet_t *rx_packet = twi_get_rx_packet();

    if(compassIsBusy | twi_is_busy())
    {
        return;
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
    }
}

float update_correction(pidReg_t *reg)
{
    twi_packet_t *rx_packet = twi_get_rx_packet();
    int16_t tmp_dev;
    static float tmp_corr = 0.0f; // todo eventuell einen sinnvollen Fehlerwert zurückgeben, sonst wird der alte wert zurückgegeben
    
    if(compassIsBusy | twi_is_busy())
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
}

void update_bat(void)
{
    float bat_voltage;
    char tmp[7];
    
    if ((getTicks() - ul_ticks_bat) > BAT_UPDATE_RATE)
    {
        ul_ticks_bat = getTicks();
        
        bat_voltage = (float)sensor_values.bat_volt / 16.0;
        if (bat_voltage >= 10)
        {
            sprintf(tmp, "%3.1fV", bat_voltage);
        }
        else
        {
            sprintf(tmp, " %3.1fV", bat_voltage);
        }
        
        lcd_print_s(1, 15, tmp);
    }
}

void check_bat(void)
{
    float bat_voltage;
    
    if ((getTicks() - ul_ticks_check_bat) > BAT_UPDATE_RATE)
    {
        ul_ticks_check_bat = getTicks();
        
        bat_voltage = (float)sensor_values.bat_volt / 16.0;
        
        if (bat_voltage < 10.5)
        {
            ioport_set_pin_level(BAT_LED, 1);
        }
        else
        {
            ioport_set_pin_level(BAT_LED, 0);
        }
    }
}

float my_atan2(int16_t x, int16_t y)
{
    float atan_result = INFINITY;
    
    if (x == 0 && y == 0)
    {
        return atan_result;
    }
    
    if (x > 0 && y == 0)
    {
        atan_result = 2.0f * M_PI;
        return atan_result;
    }
    
    if (x < 0 && y == 0)
    {
        atan_result = M_PI;
        return atan_result;
    }
    
    if (x == 0 && y > 0)
    {
        atan_result = M_PI_2;
        return atan_result;
    }
    
    if (x == 0 && y < 0)
    {
        atan_result = 3.0f * M_PI / 2.0f;
        return atan_result;
    }
    
    if (x > 0 && y > 0)
    {
        atan_result = atan(y / x);
        return atan_result;
    }
    
    if (x < 0)
    {
        atan_result = atan(y / x) + M_PI;
        return atan_result;
    }
    
    if (x > 0 && y < 0)
    {
        atan_result = atan(x / y) + 2.0f * M_PI;
        return atan_result;
    }
    
    return atan_result;
}

/************************************************************
* Local Functions                                           *
************************************************************/
static void compass_callback(void)
{
    compassIsBusy = false;
}
#endif
