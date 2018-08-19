/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: NoOS                                                           */
/* Created: 01.07.18                                                    */
/************************************************************************/

#include "asf.h"
#include "board.h"
#include "conf_board.h"

void board_init(void)
{
	#ifndef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;
	#endif
	
	ioport_init();
	
	/* Configure LED pins */
	ioport_set_pin_dir(LED_ONBOARD, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED_ONBOARD, 0);
	ioport_set_pin_dir(LED_S1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED_S1, 0);
	ioport_set_pin_dir(LED_S2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED_S2, 0);
	ioport_set_pin_dir(LED_S3, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED_S3, 0);
	
	/* Configure SPI pins */
	ioport_set_pin_mode(SPI0_MISO, IOPORT_MODE_MUX_A);
	ioport_disable_pin(SPI0_MISO);
	ioport_set_pin_mode(SPI0_MOSI, IOPORT_MODE_MUX_A);
	ioport_disable_pin(SPI0_MOSI);
	ioport_set_pin_mode(SPI0_SPCK, IOPORT_MODE_MUX_A);
	ioport_disable_pin(SPI0_SPCK);
	ioport_set_pin_mode(SPI0_NPCS0, IOPORT_MODE_MUX_A);
	ioport_disable_pin(SPI0_NPCS0);
}
