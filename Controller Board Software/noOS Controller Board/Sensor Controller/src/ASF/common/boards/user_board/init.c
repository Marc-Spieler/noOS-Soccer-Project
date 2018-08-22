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
    
    
    
    /* Configure Line Sensor pins */
    ioport_set_pin_mode(LINE1_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE2_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE3_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE4_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE5_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE6_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE7_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE8_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE9_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE10_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE11_B, LINE_FLAGS);
    ioport_set_pin_mode(LINE12_B, LINE_FLAGS);
    
    ioport_set_pin_mode(LINE1_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE2_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE3_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE4_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE5_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE6_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE7_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE8_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE9_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE10_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE11_W, LINE_FLAGS);
    ioport_set_pin_mode(LINE12_W, LINE_FLAGS);
    
    /* Configure ADC */
    pmc_enable_periph_clk(ID_ADC);
    adc_init(ADC, sysclk_get_cpu_hz(), 6400000, ADC_STARTUP_TIME_4);
    adc_configure_timing(ADC, 1, ADC_SETTLING_TIME_3, 1);
    adc_enable_channel(ADC, BATTERY_VOLTAGE);
    adc_enable_channel(ADC, LIGHT_BARRIER_RX);
    adc_configure_trigger(ADC, ADC_TRIG_SW, 0);
}
