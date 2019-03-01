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

/* PWM frequency in Hz */
#define PWM_FREQUENCY                 50
/* Period value of PWM output waveform */
#define PERIOD_VALUE                  20000
/* Initial duty cycle value */
#define INIT_DUTY_VALUE               10000

/* Signal LED pin definitions */
#define LED_ONBOARD						        PIO_PD3_IDX
#define LED_S1  						        PIO_PD2_IDX
#define LED_S2  						        PIO_PD1_IDX
#define LED_S3   						        PIO_PD0_IDX

/* SPI0 pin definition */
#define SPI0_MISO				            	PIO_PA25_IDX
#define SPI0_MOSI				            	PIO_PA26_IDX
#define SPI0_SPCK				            	PIO_PA27_IDX
#define SPI0_NPCS0				          		PIO_PA28_IDX

/* ADC pin definition */
#define BATTERY_VOLTAGE                         ADC_CHANNEL_1
#define LIGHT_BARRIER_RX                        ADC_CHANNEL_2

/* DAC pin definition */
#define DACC_CHANNEL_VREF_BLACK                 0
#define DACC_CHANNEL_VREF_WHITE                 1
#define DACC_ANALOG_CONTROL                     (DACC_ACR_IBCTLCH0(0x02) \
                                                | DACC_ACR_IBCTLCH1(0x02) \
                                                | DACC_ACR_IBCTLDACCORE(0x01))

/* Line sensor pin definition */
#define LINE_FLAGS                              (IOPORT_DIR_INPUT | IOPORT_MODE_PULLUP)

#define LINE1_B                                 PIO_PC18_IDX
#define LINE2_B                                 PIO_PC29_IDX
#define LINE3_B                                 PIO_PC10_IDX
#define LINE4_B                                 PIO_PC21_IDX
#define LINE5_B                                 PIO_PC23_IDX
#define LINE6_B                                 PIO_PC26_IDX
#define LINE7_B                                 PIO_PC1_IDX
#define LINE8_B                                 PIO_PC5_IDX
#define LINE9_B                                 PIO_PC9_IDX
#define LINE10_B                                PIO_PC12_IDX
#define LINE11_B                                PIO_PC14_IDX
#define LINE12_B                                PIO_PC16_IDX

#define LINE1_W                                 PIO_PC17_IDX
#define LINE2_W                                 PIO_PC19_IDX
#define LINE3_W                                 PIO_PC30_IDX
#define LINE4_W                                 PIO_PC20_IDX
#define LINE5_W                                 PIO_PC22_IDX
#define LINE6_W                                 PIO_PC24_IDX
#define LINE7_W                                 PIO_PC27_IDX
#define LINE8_W                                 PIO_PC3_IDX
#define LINE9_W                                 PIO_PC7_IDX
#define LINE10_W                                PIO_PC11_IDX
#define LINE11_W                                PIO_PC13_IDX
#define LINE12_W                                PIO_PC15_IDX

/* Distance sensor pin definition */
#define DFRONT_TRIG            			        PWM_CHANNEL_0
#define DFRONT_TRIG_PIN					        PIO_PC2_IDX
#define DFRONT_ECHO                             PIO_PA2_IDX 

#define DRIGHT_TRIG            			        PWM_CHANNEL_2
#define DRIGHT_TRIG_PIN					        PIO_PC6_IDX
#define DRIGHT_ECHO                             PIO_PC25_IDX

#define DREAR_TRIG           				    PWM_CHANNEL_3
#define DREAR_TRIG_PIN					        PIO_PC8_IDX
#define DREAR_ECHO                              PIO_PC28_IDX

#define DLEFT_TRIG           				    PWM_CHANNEL_1
#define DLEFT_TRIG_PIN					        PIO_PC4_IDX
#define DLEFT_ECHO                              PIO_PA5_IDX

#endif
