/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: NoOS                                                           */
/* Created: 01.07.2018                                                  */
/************************************************************************/

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

/* Board oscillator settings */
#define BOARD_FREQ_SLCK_XTAL                    (32768U)
#define BOARD_FREQ_SLCK_BYPASS                  (32768U)
#define BOARD_FREQ_MAINCK_XTAL                  (12000000U)
#define BOARD_FREQ_MAINCK_BYPASS                (12000000U)

/* Master clock frequency */
#define BOARD_MCK                               (CHIP_FREQ_CPU_MAX)
#define BOARD_NO_32K_XTAL

/* board main clock xtal startup time */
#define BOARD_OSC_STARTUP_US                    (15625)

/* Family definition (already defined) */
#define sam3x

/* Core definition */
#define cortexm3

/* PWM settings */
#define PWM_FREQUENCY                           (20000)
#define PERIOD_VALUE                            (1000)
#define INIT_DUTY_VALUE                         (500)

/** Enable SD MMC interface pins through HSMCI */
#define CONF_BOARD_SD_MMC_HSMCI
#define SD_MMC_HSMCI_MEM_CNT                    (1)
#define SD_MMC_HSMCI_SLOT_0_SIZE                (4)

/* Signal LED pin definition */
#define LED_ONBOARD                             (PIO_PD10_IDX)
#define LED_M1                                  (PIO_PD7_IDX)
#define LED_M2                                  (PIO_PD8_IDX)
#define LED_M3                                  (PIO_PD9_IDX)
#define LED_BAT                                 (PIO_PD6_IDX)

/* Battery warner pin definition */
/*#define BAT_WARN                                (PWM_CHANNEL_3)
#define BAT_WARN_PIN                            (PIO_PC8_IDX)*/

/* Pushbutton pin definition */
#define PB_UP                                   (PIO_PD0_IDX)
#define PB_LEFT                                 (PIO_PD1_IDX)
#define PB_MID                                  (PIO_PD2_IDX)
#define PB_RIGHT                                (PIO_PD3_IDX)
#define PB_DOWN                                 (PIO_PD4_IDX)
#define PB_RETURN                               (PIO_PD5_IDX)

/* Raspberry Pi GPIO pin definition */
#define RPI1                                    (PIO_PA1_IDX)
//#define RPI2                                    (PIO_PA8_IDX)
//#define RPI3                                    (PIO_PA9_IDX)
#define RPI4                                    (PIO_PA7_IDX)
#define RPI5                                    (PIO_PA5_IDX)
#define RPI6                                    (PIO_PA0_IDX)

/* Motor pin definition */
#define MOTOR_LEFT           				    (PWM_CHANNEL_0)
#define MOTOR_LEFT_L					        (PIO_PC2_IDX)
#define MOTOR_LEFT_H				            (PIO_PC3_IDX)
#define MOTOR_RIGHT                             (PWM_CHANNEL_1)
#define MOTOR_RIGHT_L                           (PIO_PC4_IDX)
#define MOTOR_RIGHT_H                           (PIO_PC5_IDX)
#define MOTOR_REAR                              (PWM_CHANNEL_2)
#define MOTOR_REAR_L                            (PIO_PC6_IDX)
#define MOTOR_REAR_H                            (PIO_PC7_IDX)

/* Encoder pin definition */
#define ENC_CLK                                 (PWM_CHANNEL_4)
#define ENC_CLK_PIN                             (PIO_PC21_IDX)
#define ENC_LOAD                                (PIO_PB0_IDX)

#define ENC_LEFT_A						        (PIO_PC24_IDX)
#define ENC_LEFT_B						        (PIO_PC25_IDX)
#define ENC_LEFT_C						        (PIO_PC26_IDX)
#define ENC_LEFT_D						        (PIO_PC27_IDX)
#define ENC_LEFT_E						        (PIO_PC28_IDX)
#define ENC_LEFT_F						        (PIO_PC29_IDX)
#define ENC_LEFT_G						        (PIO_PC30_IDX)

#define ENC_RIGHT_A						        (PIO_PC16_IDX)
#define ENC_RIGHT_B						        (PIO_PC17_IDX)
#define ENC_RIGHT_C						        (PIO_PC18_IDX)
#define ENC_RIGHT_D                             (PIO_PC19_IDX)
#define ENC_RIGHT_E						        (PIO_PC20_IDX)
#define ENC_RIGHT_F						        (PIO_PC22_IDX)
#define ENC_RIGHT_G						        (PIO_PC23_IDX)

#define ENC_REAR_A						        (PIO_PC1_IDX)
#define ENC_REAR_B					            (PIO_PC10_IDX)
#define ENC_REAR_C						        (PIO_PC11_IDX)
#define ENC_REAR_D						        (PIO_PC12_IDX)
#define ENC_REAR_E						        (PIO_PC13_IDX)
#define ENC_REAR_F						        (PIO_PC14_IDX)
#define ENC_REAR_G						        (PIO_PC15_IDX)

/* UART pin definition */
#define UART_RX                                 (PIO_PA10_IDX)
#define UART_TX                                 (PIO_PA11_IDX)

/* I2C0 pin definition */
#define I2C0_DATA                               (PIO_PA17_IDX)
#define I2C0_CLK                                (PIO_PA18_IDX)

/* SPI1 pin definition */
#define SPI1_MISO                               (PIO_PA12_IDX)
#define SPI1_MOSI                               (PIO_PA13_IDX)
#define SPI1_SPCK                               (PIO_PA16_IDX)
#define SPI1_NPCS0                              (PIO_PA14_IDX)

/* micro SD Card pin definition */
#define MSD_CMD                                 (PIO_PA20_IDX)
#define MSD_CLK                                 (PIO_PA19_IDX)
#define MSD_DATA0                               (PIO_PA21_IDX)
#define MSD_DATA1                               (PIO_PA22_IDX)
#define MSD_DATA2                               (PIO_PA23_IDX)
#define MSD_DATA3                               (PIO_PA24_IDX)

#endif

#if 0

#ifndef CONF_BOARD_H_INCLUDED
#define CONF_BOARD_H_INCLUDED

/** Enable Com Port. */
#define CONF_BOARD_UART_CONSOLE

//! [tc_define_peripheral]
/* Use TC Peripheral 0. */
#define TC             TC0
#define TC_PERIPHERAL  0
//! [tc_define_peripheral]

//! [tc_define_ch1]
/* Configure TC0 channel 1 as waveform output. */
#define TC_CHANNEL_WAVEFORM 1
#define ID_TC_WAVEFORM      ID_TC1
#define PIN_TC_WAVEFORM     PIN_TC0_TIOA1
#define PIN_TC_WAVEFORM_MUX PIN_TC0_TIOA1_MUX
//! [tc_define_ch1]

//! [tc_define_ch2]
/* Configure TC0 channel 2 as capture input. */
#define TC_CHANNEL_CAPTURE 2
#define ID_TC_CAPTURE ID_TC2
#define PIN_TC_CAPTURE PIN_TC0_TIOA2
#define PIN_TC_CAPTURE_MUX PIN_TC0_TIOA2_MUX
//! [tc_define_ch2]

//! [tc_define_irq_handler]
/* Use TC2_Handler for TC capture interrupt. */
#define TC_Handler  TC2_Handler
#define TC_IRQn     TC2_IRQn
//! [tc_define_irq_handler]

#endif /* CONF_BOARD_H_INCLUDED */

#endif