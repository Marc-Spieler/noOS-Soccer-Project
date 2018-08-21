/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: NoOS                                                           */
/* Created: 01.07.18                                                    */
/************************************************************************/

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

/* Board oscillator settings */
#define BOARD_FREQ_SLCK_XTAL                    (32768U)
#define BOARD_FREQ_SLCK_BYPASS                  (32768U)
#define BOARD_FREQ_MAINCK_XTAL                  (12000000U)
#define BOARD_FREQ_MAINCK_BYPASS                (12000000U)

/* Master clock frequency */
#define BOARD_MCK                               CHIP_FREQ_CPU_MAX
#define BOARD_NO_32K_XTAL

/* board main clock xtal startup time */
#define BOARD_OSC_STARTUP_US                    15625

/* Family definition (already defined) */
#define sam3x

/* Core definition */
#define cortexm3

/* Signal LED pin definitions */
#define LED_ONBOARD                             PIO_PD10_IDX
#define LED_M1                                  PIO_PD7_IDX
#define LED_M2                                  PIO_PD8_IDX
#define LED_M3                                  PIO_PD9_IDX
#define LED_BAT                                 PIO_PD6_IDX

/* Pushbutton pin definitions */
#define PB_UP                                   PIO_PD0_IDX
#define PB_LEFT                                 PIO_PD1_IDX
#define PB_MID                                  PIO_PD2_IDX
#define PB_RIGHT                                PIO_PD3_IDX
#define PB_DOWN                                 PIO_PD4_IDX
#define PB_RETURN                               PIO_PD5_IDX

/* I2C0 pin definition */
#define I2C0_DATA                               PIO_PA17_IDX
#define I2C0_CLK                                PIO_PA18_IDX

/* SPI1 pin definition */
#define SPI1_MISO                               PIO_PA12_IDX
#define SPI1_MOSI                               PIO_PA13_IDX
#define SPI1_SPCK                               PIO_PA16_IDX
#define SPI1_NPCS0                              PIO_PA14_IDX

/* micro SD Card pin definition */
#define MSD_CMD                                 PIO_PA20_IDX
#define MSD_CLK                                 PIO_PA19_IDX
#define MSD_DATA0                               PIO_PA21_IDX
#define MSD_DATA1                               PIO_PA22_IDX
#define MSD_DATA2                               PIO_PA23_IDX
#define MSD_DATA3                               PIO_PA24_IDX

#endif
