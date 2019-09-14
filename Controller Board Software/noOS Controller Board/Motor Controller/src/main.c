/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.07.2018                                                  */
/************************************************************************/

#include "asf.h"
#include "string.h"
#include "timing.h"
#include "lcd.h"
#include "menu.h"
#include "comm.h"
#include "compass.h"
#include "sd.h"
#include "motor.h"
#include "iniparser.h"
#include "support.h"
#include "math.h"
#include "bt.h"

//#define RX_BUF_MAX_SIZE 128

uint8_t trn = 5;

/*static uint8_t rxId = 0;
static uint8_t rxBuf[RX_BUF_MAX_SIZE];
static Bool okReceived = false;
static const char *btEventTable[] = BT_EVENT_TABLE;*/

void noOS_bootup_sequence(void);
/*static void process_rx_packet(void);
static int8_t find_str(const char *s, const char *t);*/

int main(void)
{
    event_t act_event;

    sysclk_init();

    board_init();
    SysTick_Config(sysclk_get_cpu_hz() / 1000);
    
    motor_init();

    lcd_init();
    compass_init();

    bt_init();
    
    sd_mmc_init();
    sd_init();
    parse_ini_file();
    
    spi_init();
    
    uint32_t bt_tx_ticks = 0;
    
    while (1)
    {
        update_comm();
        update_compass();
        update_heartbeat();
        check_battery();
        
        prepare_values_to_send();
        process_new_sensor_values();
        //process_bt_rx();

        act_event = button_events();
        menu(act_event);
        
        if((getTicks() - bt_rx_ticks) <= 100)
        {
            ioport_set_pin_level(LED_ONBOARD, true);
        }
        else
        {
            ioport_set_pin_level(LED_ONBOARD, false);
        }
        
        if((getTicks() - bt_tx_ticks) >= 5)
        {
            bt_tx_ticks = getTicks();
            char sprintf_buf[14];
            sprintf(sprintf_buf, "%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d\r\n", s.line.single.segment_1, s.line.single.segment_2, s.line.single.segment_3,\
                    s.line.single.segment_4, s.line.single.segment_5, s.line.single.segment_6, s.line.single.segment_7, s.line.single.segment_8,\
                    s.line.single.segment_9, s.line.single.segment_10, s.line.single.segment_11, s.line.single.segment_12);
            bt_write(sprintf_buf, 14);
        }
    }
}

/*!\brief Process incoming event
 *
 * This function processes the incoming
 * event from the bluetooth module.
 *
 * \param none
 * \return none
 */
/*static void process_rx_packet(void)
{
    int pos;

    // OK event
    if(find_str((char *)rxBuf, btEventTable[BT_EVENT_OK_ID]) >= 0)
    {
        okReceived = true;
        // bt_inst.tx.cmd = BT_NO_CMD_ID;
    }

    // Reset receive buffer
    rxId = 0;
    rxBuf[rxId] = '\0';
}*/

/*!\brief Find string within string.
 *
 * Find a string within a string
 *
 * \param1 pointer to string
 * \param2 pointer to string
 * \return position
 */
/*static int8_t find_str(const char *s, const char *t)
{
    int rval = -1;
    int p = 0;
    const char *a;
    const char *b;

    for (; *s != '\0'; s++, p++)
    {
        a = s;
        b = t;
        for (; *a == *b; a++, b++)
        {
            if (*a == '\0')
            {
                rval = -1;
                break;
            };
        };
        if (*b == '\0')
        {
            rval = p;
            break;
        };
    };
    return rval;
}*/
