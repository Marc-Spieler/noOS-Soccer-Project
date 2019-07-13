/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.07.2019                                                  */
/************************************************************************/

#include "bt.h"

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
    uart_enable(UART);
}
