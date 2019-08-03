/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.07.2019                                                  */
/************************************************************************/

#include "bt.h"

#define RX_BUF_MAX_SIZE 128

static uint8_t txlen = 0;
static uint8_t txid = 0;
static uint8_t *txbuf = NULL;
static uint8_t rxid = 0;
static uint8_t rxbuf[RX_BUF_MAX_SIZE];

void bt_init(void)
{
    pmc_enable_periph_clk(ID_UART);
    uart_disable(UART);

    sam_uart_opt_t uart_settings =
    {
        .ul_mck = BOARD_MCK,
        .ul_baudrate = 9600,
        .ul_mode = (UART_MR_CHMODE_NORMAL | UART_MR_PAR_NO)
    };

    uart_init(UART, &uart_settings);
    uart_enable_interrupt(UART, UART_IER_RXBUFF);
    uart_enable(UART);
}

void bt_write(uint8_t *pbuf, uint8_t len)
{
    txlen = len;
    txid = 0;
    txbuf = pbuf;
    uart_write(UART, txbuf[txid++]);
    uart_enable_interrupt(UART, UART_IER_TXBUFE);
}

uint8_t bt_read(uint8_t *pbuf, uint8_t maxcount)
{

}

void UART_Handler(void)
{
    static uint32_t ul_status;

    ul_status = uart_get_status(UART);
    
    if (ul_status & UART_SR_TXBUFE)
    {
        if(txid < txlen)
        {
            uart_write(UART, txbuf[txid++]);
        }
        else
        {
            uart_disable_interrupt(UART, UART_IER_TXBUFE);
        }
    }

    if (ul_status & UART_SR_RXBUFF)
    {
        uart_read(UART, &rxbuf[rxid++]);
        if(rxid >= RX_BUF_MAX_SIZE)
        {
            rxid = 0;
        }
    }
}

