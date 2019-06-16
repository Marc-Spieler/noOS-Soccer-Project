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

/* PWM settings */
#define PWM_FREQUENCY                           (50)
#define PERIOD_VALUE                            (20000)
#define INIT_DUTY_VALUE                         (10000)

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

/* ADC pin definition */
#define BATTERY_VOLTAGE                         (ADC_CHANNEL_1)

/* DAC pin definition */
#define DACC_CHANNEL_LINE_VREF                  0
#define DACC_ANALOG_CONTROL                     (DACC_ACR_IBCTLCH0(0x02) \
                                                | DACC_ACR_IBCTLCH1(0x02) \
                                                | DACC_ACR_IBCTLDACCORE(0x01))

/* Line sensor pin definition */
#define LINE_FLAGS                              (IOPORT_DIR_INPUT | IOPORT_MODE_PULLUP)

#define LINE1                                   (PIO_PC17_IDX)
#define LINE2                                   (PIO_PC19_IDX)
#define LINE3                                   (PIO_PC30_IDX)
#define LINE4                                   (PIO_PC20_IDX)
#define LINE5                                   (PIO_PC22_IDX)
#define LINE6                                   (PIO_PC24_IDX)
#define LINE7                                   (PIO_PC27_IDX)
#define LINE8                                   (PIO_PC3_IDX)
#define LINE9                                   (PIO_PC7_IDX)
#define LINE10                                  (PIO_PC11_IDX)
#define LINE11                                  (PIO_PC13_IDX)
#define LINE12                                  (PIO_PC15_IDX)

/* Distance sensor pin definition */
#define DISTANCE_TRIG                           (PWM_CHANNEL_0)
#define DISTANCE_TRIG_PIN				        (PIO_PC2_IDX)
#define DISTANCE_ECHO                           (PIO_PA2_IDX)

#endif
