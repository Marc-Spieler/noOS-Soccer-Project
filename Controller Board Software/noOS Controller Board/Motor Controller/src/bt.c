/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.07.2019                                                  */
/************************************************************************/

#include "bt.h"

#define RX_BUF_MAX_SIZE 128

static uint8_t txLen = 0;
static uint8_t txId = 0;
static uint8_t *txBuf = NULL;
static uint8_t rxWrId = 0;
static uint8_t rxRdId = 0;
static uint8_t rxBuf[RX_BUF_MAX_SIZE];

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
    uart_enable_interrupt(UART, UART_IER_RXRDY);
    NVIC_EnableIRQ(UART_IRQn);
    uart_enable(UART);
}

void bt_write_string(uint8_t *pbuf, uint8_t len)
{
    txLen = len;
    txId = 0;
    txBuf = pbuf;
    uart_write(UART, txBuf[txId++]);
    uart_enable_interrupt(UART, UART_IER_TXRDY);
}

uint8_t bt_read_byte(uint8_t *pbuf)
{
    // if buffer is empty then return immediately
    if(rxRdId == rxWrId)
    {
        return 0;
    }

    *pbuf = rxBuf[rxRdId++];
    if(rxRdId >= RX_BUF_MAX_SIZE)
    {
        rxRdId = 0;
    }

    return 1;
}

void UART_Handler(void)
{
    uint32_t ul_status = uart_get_status(UART);

    if (ul_status & UART_SR_TXRDY)
    {
        if(txId < txLen)
        {
            uart_write(UART, txBuf[txId++]);
        }
        else
        {
            uart_disable_interrupt(UART, UART_IDR_TXRDY);
        }
    }

    if (ul_status & UART_SR_RXRDY)
    {
        uart_read(UART, &rxBuf[rxWrId++]);
        if(rxWrId >= RX_BUF_MAX_SIZE)
        {
            rxWrId = 0;
        }
    }
}

