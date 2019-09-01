/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.07.2019                                                  */
/************************************************************************/

#include "bt.h"

//#define RX_BUF_MAX_SIZE 128

static uint8_t txLen = 0;
static uint8_t txId = 0;
static uint8_t *txBuf = NULL;
/*static uint8_t rxWrId = 0;
static uint8_t rxRdId = 0;
static uint8_t rxBuf[RX_BUF_MAX_SIZE];*/

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
}

void bt_write_string(uint8_t *pbuf, uint8_t len)
{
    txLen = len;
    txId = 0;
    txBuf = pbuf;
    usart_write(USART0, txBuf[txId++]);
    usart_enable_interrupt(USART0, US_IER_TXRDY);
}

/*uint8_t bt_read_byte(uint8_t *pbuf)
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
}*/

#if 0
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
        /*usart_read(USART0, &rxBuf[rxWrId++]);
        if(rxWrId >= RX_BUF_MAX_SIZE)
        {
            rxWrId = 0;
        }*/
    }
}
#endif