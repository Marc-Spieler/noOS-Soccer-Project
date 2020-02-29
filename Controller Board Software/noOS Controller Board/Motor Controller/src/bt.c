/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.07.2019                                                  */
/************************************************************************/

#include "bt.h"
#include "timing.h"
#include "pid.h"
#include "comm.h"

bt_rx_t bt_rx;
bt_tx_t bt_tx;

static uint8_t txLen = 0;
static uint8_t txId = 0;
static uint8_t *txBuf = NULL;
static uint8_t rx_chunk = 0;

uint32_t bt_rx_ticks = 0;
uint32_t bt_tx_ticks = 0;

void bt_init(void)
{
    pmc_enable_periph_clk(ID_USART0);
    usart_disable_rx(USART0);
    usart_disable_tx(USART0);
    
    sam_usart_opt_t uart_settings =
    {
        .baudrate = 38400,
        .char_length = US_MR_CHRL_8_BIT,
        .parity_type = US_MR_PAR_NO,
        .stop_bits = US_MR_NBSTOP_1_BIT,
        .channel_mode = US_MR_CHMODE_NORMAL,
        .irda_filter = 0
    };
    
    usart_init_rs232(USART0, &uart_settings, sysclk_get_peripheral_hz());
    usart_enable_interrupt(USART0, US_IER_RXRDY);
    NVIC_EnableIRQ(USART0_IRQn);
    usart_enable_rx(USART0);
    usart_enable_tx(USART0);
	
	bt_tx.sbyte.sbit = true;
}

void bt_write(uint8_t *pbuf, uint8_t len)
{
    txLen = len;
    txId = 0;
    txBuf = pbuf;
    usart_write(USART0, txBuf[txId++]);
    usart_enable_interrupt(USART0, US_IER_TXRDY);
}

void bt_maintenance(void)
{
	/* transfer new BT data */
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
	
	/* set partner inactive if connection interrupts */
	if((getTicks() - bt_rx_ticks) >= 500)
	{
		bt_rx.sbyte.active = false;
	}
}

void USART0_Handler(void)
{
    uint32_t ul_status = usart_get_status(USART0);

    if (ul_status & US_CSR_TXRDY)
    {
        if(txId < txLen)
        {
            usart_write(USART0, txBuf[txId++]);
        }
        else
        {
            usart_disable_interrupt(USART0, US_IDR_TXRDY);
        }
    }

    if (ul_status & US_CSR_RXRDY)
    {
        uint8_t tmp = 0;
        usart_read(USART0, &tmp);
        
        if(tmp >= 128) rx_chunk = 1;
        
        switch(rx_chunk)
        {
            case 1:
                bt_rx.sbyte.active = (tmp & 0x2) ? 1 : 0;
                bt_rx.sbyte.at_goal = (tmp & 0x1) ? 1 : 0;
                break;
            case 2:
                bt_rx.ball_angle = tmp - 63;
                break;
            case 3:
                bt_rx.goal_angle = tmp - 63;
                break;
            case 4:
                bt_rx.ball_dist = tmp;
                break;
            case 5:
                bt_rx.goal_dist = tmp;
                /*ioport_set_pin_level(LED_M2, true);
                ioport_set_pin_level(LED_M2, false);*/
                break;
            default:
                break;
        }
        
        rx_chunk++;
        bt_rx_ticks = getTicks();
    }
}
