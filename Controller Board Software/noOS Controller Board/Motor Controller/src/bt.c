/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.07.2019                                                  */
/************************************************************************/

#include "string.h"
#include "bt.h"
#include "timing.h"
#include "pid.h"
#include "comm.h"
#include "menu.h"

bt_rx_t bt_rx;
bt_tx_t bt_tx;

static uint8_t txLen = 0;
static uint8_t txId = 0;
static uint8_t txBuf[32];
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
    memcpy(txBuf, pbuf, len);
    usart_write(USART0, txBuf[txId++]);
    usart_enable_interrupt(USART0, US_IER_TXRDY);
}

void bt_maintenance(void)
{
	/* transfer new BT data */
    #if BT_PC
        if(s.battery.percentage <= 127) bt_tx.battery_percentage = (uint8_t)(s.battery.percentage);
        bt_tx.line_part_1.ls1 = s.line.single.segment_1;
        bt_tx.line_part_1.ls2 = s.line.single.segment_2;
        bt_tx.line_part_1.ls3 = s.line.single.segment_3;
        bt_tx.line_part_1.ls4 = s.line.single.segment_4;
        bt_tx.line_part_1.ls5 = s.line.single.segment_5;
        bt_tx.line_part_1.ls6 = s.line.single.segment_6;
        bt_tx.line_part_1.ls7 = s.line.single.segment_7;
        bt_tx.line_part_2.ls8 = s.line.single.segment_8;
        bt_tx.line_part_2.ls9 = s.line.single.segment_9;
        bt_tx.line_part_2.ls10 = s.line.single.segment_10;
        bt_tx.line_part_2.ls11 = s.line.single.segment_11;
        bt_tx.line_part_2.ls12 = s.line.single.segment_12;
        bt_tx.currentLineCalibration = mts.line_cal_value;
        bt_tx.absoluteCompassPart1.bit0_6 = (1831 & 0x7f);
        bt_tx.absoluteCompassPart2.bit7_11 = ((1831 & 0xf80) >> 7);
        bt_tx.relativeCompassPart1.bit0_6 = ((uint16_t)((180.0f + 180.0f) * 10.0f) & 0x7f);
        bt_tx.relativeCompassPart2.bit7_11 = (((uint16_t)((180.0f + 180.0f) * 10.0f) & 0xf80) >> 7);
	    
	    if((getTicks() - bt_tx_ticks) >= 200)
	    {
    	    bt_tx_ticks = getTicks();
    	    
    	    bt_write((uint8_t *)&bt_tx, 9);
	    }
    #else
        bt_tx.ball_angle = (int)(s.ball.dir * 0.3556) + 63;
        bt_tx.goal_angle = (int)(s.goal.dir * 0.3356) + 63;
        bt_tx.goal_dist = s.goal.diff;
        
        if((getTicks() - bt_tx_ticks) >= 100)
        {
            bt_tx_ticks = getTicks();
            
            bt_write((uint8_t *)&bt_tx, 5);
        }
    #endif
	
	/* set partner inactive if connection interrupts */
	#if BT_PC
    #else
        if((getTicks() - bt_rx_ticks) >= 500)
	    {
		    bt_rx.sbyte.active = false;
	    }
    #endif
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
        bt_rx_ticks = getTicks();
    
        if(tmp >= 128) rx_chunk = 1;
        
        #if BT_PC
            switch(rx_chunk)
            {
                case 1:
                    if(tmp & 0x01)
                    {
                        ioport_set_pin_level(LED_ONBOARD, true);
                    }
                    else
                    {
                        ioport_set_pin_level(LED_ONBOARD, false);
                    }
                    
                    if(tmp & 0x02)
                    {
                        bt_rx.sbyte.setLineCalibration = true;
                    }
                    else
                    {
                        bt_rx.sbyte.setLineCalibration = false;
                    }
                    
                    if(tmp & 0x04)
                    {
                        bt_rx.sbyte.startAction = true;
                    }
                    else
                    {
                        bt_rx.sbyte.startAction = false;
                    }
                    break;
                case 2:
                    if(bt_rx.sbyte.setLineCalibration)
                    {
                        mts.line_cal_value = tmp;
                        // save new value to sd card
                    }
                default:
                    break;
            }
        #else
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
                    break;
                default:
                    break;
            }
        #endif
        
        rx_chunk++;
    }
}
