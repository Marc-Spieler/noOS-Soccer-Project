/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: NoOS                                                           */
/* Created: 01.07.18                                                    */
/************************************************************************/

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

/* Board oscillator settings */
#define BOARD_FREQ_SLCK_XTAL          (32768U)
#define BOARD_FREQ_SLCK_BYPASS        (32768U)
#define BOARD_FREQ_MAINCK_XTAL        (12000000U)
#define BOARD_FREQ_MAINCK_BYPASS      (12000000U)

/* Master clock frequency */
#define BOARD_MCK                     CHIP_FREQ_CPU_MAX
#define BOARD_NO_32K_XTAL

/* board main clock xtal startup time */
#define BOARD_OSC_STARTUP_US          15625

/* Name of the board */
//#define BOARD_NAME						      "Soccer 2v2 Motor Controller"

/* Board definition */
//#define soccer2v2motorctrl

/* Family definition (already defined) */
#define sam3x

/* Core definition */
#define cortexm3

/* Signal LED pin definitions */
#define LED_ONBOARD						        (PIO_PD3_IDX)
#define LED_S1  						        (PIO_PD2_IDX)
#define LED_S2  						        (PIO_PD1_IDX)
#define LED_S3   						        (PIO_PD0_IDX)

/* SPI0 pin definition */
#define SPI0_MISO				            	(PIO_PA25_IDX)
#define SPI0_MOSI				            	(PIO_PA26_IDX)
#define SPI0_SPCK				            	(PIO_PA27_IDX)
#define SPI0_NPCS0				          		(PIO_PA28_IDX)

#endif // CONF_BOARD_H
