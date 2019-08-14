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

#define RX_BUF_MAX_SIZE 128

uint8_t trn = 5;

static uint8_t rxId = 0;
static uint8_t rxBuf[RX_BUF_MAX_SIZE];
static Bool okReceived = false;
static const char *btEventTable[] = BT_EVENT_TABLE;

void noOS_bootup_sequence(void);
static void process_rx_packet(void);
static int8_t find_str(const char *s, const char *t);

int main(void)
{
    event_t act_event;
    uint8_t c;

    sysclk_init();

    board_init();
    SysTick_Config(sysclk_get_cpu_hz() / 1000);
    
    /*motor_init();
    //init_battery_warning();

    compass_init();
    lcd_init();

    sd_mmc_init();
    sd_init();
    //write_time_test_2();
    //create_default_ini_file();
    parse_ini_file();
    spi_init();*/

    bt_init();

    mdelay(500);
    
    bt_write_string("^^^", 3);
    
    mdelay(500);
    
    // send AT&F*
    bt_write_string("AT&F*\r\n", 7);

    //while(!okReceived)
    //{
        /*
        * As long as there are characters in the receive queue
        * copy characters into buffer and increment buffer index
        */
        while(bt_read_byte(&c) > 0)
        {
            // Copy character into buffer
            rxBuf[rxId++] = c;

            // Increment buffer index and terminate string with NULL
            // If buffer is full then roll over and start form the beginning.
            if(rxId >= RX_BUF_MAX_SIZE)
            {
                rxId = 0;
            }
            rxBuf[rxId] = '\0';

            // Check character for carriage return
            if(c == '\r')
            {
                process_rx_packet();
            }
        };
    //};
    okReceived = false;
    
    // send ATS102=1
    bt_write_string("ATS102=1\r\n", 10);
    mdelay(500);

    // send ATS320=1
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'A');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'T');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'S');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'3');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'2');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'0');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'=');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'1');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\r');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\n');
    mdelay(500);

    // send ATS321=3
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'A');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'T');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'S');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'3');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'2');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'1');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'=');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'3');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\r');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\n');
    mdelay(500);

    // send ATS0=1
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'A');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'T');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'S');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'0');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'=');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'1');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\r');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\n');
    mdelay(500);
    
    // send AT&W
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'A');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'T');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'&');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'W');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\r');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\n');
    mdelay(500);
    
    // send ATZ
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'A');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'T');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'Z');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\r');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\n');
    mdelay(500);

    // send ATI4
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'A');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'T');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'I');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'4');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\r');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\n');
    mdelay(500);

    // send AT+BTP
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'A');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'T');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'+');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'B');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'T');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'P');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\r');
    while(!uart_is_tx_ready(UART));
    while(!uart_is_tx_empty(UART));
    uart_write(UART, (uint8_t)'\n');
    
    ioport_set_pin_level(LED_ONBOARD, 1);
    mdelay(250);
    ioport_set_pin_level(LED_ONBOARD, 0);
    /*mdelay(10000);
    
    
    
    ioport_set_pin_level(LED_ONBOARD, 1);
    mdelay(250);
    ioport_set_pin_level(LED_ONBOARD, 0);
    mdelay(250);*/
    
    uint8_t counter = 0x30;
    
    while (1)
    {
        bt_write_string(&counter, 1);
        //bt_write_string("\r\n", 2);
        counter++;
        
        if(counter >= 0x3a)
        {
            counter = 0x30;
			ioport_set_pin_level(LED_ONBOARD, 1);
        }
		else
		{
			ioport_set_pin_level(LED_ONBOARD, 0);
		}
        
        mdelay(500);
        /*update_comm();
        update_compass();
        update_heartbeat();
        check_battery();
        
        prepare_values_to_send();
        process_new_sensor_values();

        act_event = button_events();
        menu(act_event);*/
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
static void process_rx_packet(void)
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
}

/*!\brief Find string within string.
 *
 * Find a string within a string
 *
 * \param1 pointer to string
 * \param2 pointer to string
 * \return position
 */
static int8_t find_str(const char *s, const char *t)
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
}
