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
    bt_tx.sbyte.sbit = true;
    
    while (1)
    {
        update_comm();
        update_compass();
        update_heartbeat();
        check_battery();
        
        prepare_values_to_send();
        process_new_sensor_values();

        act_event = button_events();
        menu(act_event);
        
        bt_tx.sbyte.at_goal = s.distance.two.arrived;
        bt_tx.ball_angle = (int)(s.ball.dir / 2.812) + 63;
        bt_tx.goal_angle = (int)(s.goal.dir / 2.812) + 63;
        bt_tx.goal_dist = s.goal.diff;
        
        if((getTicks() - bt_tx_ticks) >= 100)
        {
            bt_tx_ticks = getTicks();
            
            uint8_t btbuf[5];
            
            btbuf[0] = bt_tx.full_sbyte;
            btbuf[1] = bt_tx.ball_angle;
            btbuf[2] = bt_tx.goal_angle;
            btbuf[3] = bt_tx.ball_dist;
            btbuf[4] = bt_tx.goal_dist;
            
            bt_write(&btbuf, 5);
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
