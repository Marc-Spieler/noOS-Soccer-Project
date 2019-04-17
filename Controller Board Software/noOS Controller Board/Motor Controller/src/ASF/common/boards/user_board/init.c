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
	ioport_set_pin_dir(LED_M1, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED_M1, 0);
	ioport_set_pin_dir(LED_M2, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED_M2, 0);
	ioport_set_pin_dir(LED_M3, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED_M3, 0);
	ioport_set_pin_dir(LED_BAT, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED_BAT, 0);
	
	/* Configure pushbutton pins */
	ioport_set_pin_dir(PB_UP, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PB_UP, IOPORT_MODE_PULLUP | IOPORT_MODE_DEBOUNCE);
	ioport_set_pin_dir(PB_LEFT, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PB_LEFT, IOPORT_MODE_PULLUP | IOPORT_MODE_DEBOUNCE);
	ioport_set_pin_dir(PB_MID, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PB_MID, IOPORT_MODE_PULLUP | IOPORT_MODE_DEBOUNCE);
	ioport_set_pin_dir(PB_RIGHT, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PB_RIGHT, IOPORT_MODE_PULLUP | IOPORT_MODE_DEBOUNCE);
	ioport_set_pin_dir(PB_DOWN, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PB_DOWN, IOPORT_MODE_PULLUP | IOPORT_MODE_DEBOUNCE);
	ioport_set_pin_dir(PB_RETURN, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PB_RETURN, IOPORT_MODE_PULLUP | IOPORT_MODE_DEBOUNCE);
  
    /* Configure Motor pins */
    ioport_set_pin_mode(MOTOR_LEFT_L, IOPORT_MODE_MUX_B);
    ioport_disable_pin(MOTOR_LEFT_L);
    ioport_set_pin_mode(MOTOR_LEFT_H, IOPORT_MODE_MUX_B);
    ioport_disable_pin(MOTOR_LEFT_H);
    ioport_set_pin_mode(MOTOR_RIGHT_L, IOPORT_MODE_MUX_B);
    ioport_disable_pin(MOTOR_RIGHT_L);
    ioport_set_pin_mode(MOTOR_RIGHT_H, IOPORT_MODE_MUX_B);
    ioport_disable_pin(MOTOR_RIGHT_H);
    ioport_set_pin_mode(MOTOR_REAR_L, IOPORT_MODE_MUX_B);
    ioport_disable_pin(MOTOR_REAR_L);
    ioport_set_pin_mode(MOTOR_REAR_H, IOPORT_MODE_MUX_B);
    ioport_disable_pin(MOTOR_REAR_H);

    /* Configure Encoder pins */
    ioport_set_pin_mode(ENC_CLK, IOPORT_MODE_MUX_B);
    ioport_disable_pin(ENC_CLK);
    ioport_set_pin_mode(ENC_LOAD, IOPORT_DIR_OUTPUT);
    ioport_set_pin_level(ENC_LOAD, 1);

    ioport_set_pin_mode(ENC_LEFT_A, ENC_LEFT_A_FLAGS);
    ioport_set_pin_mode(ENC_LEFT_B, ENC_LEFT_B_FLAGS);
    ioport_set_pin_mode(ENC_LEFT_C, ENC_LEFT_C_FLAGS);
    ioport_set_pin_mode(ENC_LEFT_D, ENC_LEFT_D_FLAGS);
    ioport_set_pin_mode(ENC_LEFT_E, ENC_LEFT_E_FLAGS);
    ioport_set_pin_mode(ENC_LEFT_F, ENC_LEFT_F_FLAGS);
    ioport_set_pin_mode(ENC_LEFT_G, ENC_LEFT_G_FLAGS);
    
    ioport_set_pin_mode(ENC_RIGHT_A, ENC_RIGHT_A_FLAGS);
    ioport_set_pin_mode(ENC_RIGHT_B, ENC_RIGHT_B_FLAGS);
    ioport_set_pin_mode(ENC_RIGHT_C, ENC_RIGHT_C_FLAGS);
    ioport_set_pin_mode(ENC_RIGHT_D, ENC_RIGHT_D_FLAGS);
    ioport_set_pin_mode(ENC_RIGHT_E, ENC_RIGHT_E_FLAGS);
    ioport_set_pin_mode(ENC_RIGHT_F, ENC_RIGHT_F_FLAGS);
    ioport_set_pin_mode(ENC_RIGHT_G, ENC_RIGHT_G_FLAGS);
    
    ioport_set_pin_mode(ENC_REAR_A, ENC_REAR_A_FLAGS);
    ioport_set_pin_mode(ENC_REAR_B, ENC_REAR_B_FLAGS);
    ioport_set_pin_mode(ENC_REAR_C, ENC_REAR_C_FLAGS);
    ioport_set_pin_mode(ENC_REAR_D, ENC_REAR_D_FLAGS);
    ioport_set_pin_mode(ENC_REAR_E, ENC_REAR_E_FLAGS);
    ioport_set_pin_mode(ENC_REAR_F, ENC_REAR_F_FLAGS);
    ioport_set_pin_mode(ENC_REAR_G, ENC_REAR_G_FLAGS);
  
    /* Configure Raspberry Pi GPIO pins */
    ioport_set_pin_dir(RPI1, IOPORT_DIR_OUTPUT);
    ioport_set_pin_level(RPI1, 1);
    ioport_set_pin_dir(RPI2, IOPORT_DIR_INPUT);
    ioport_set_pin_dir(RPI3, IOPORT_DIR_INPUT);
    ioport_set_pin_dir(RPI4, IOPORT_DIR_INPUT);
    ioport_set_pin_dir(RPI5, IOPORT_DIR_INPUT);
    ioport_set_pin_dir(RPI6, IOPORT_DIR_INPUT);
  
    /* Configure I2C pins */
    ioport_set_pin_mode(I2C0_DATA, IOPORT_MODE_MUX_A);
    ioport_disable_pin(I2C0_DATA);
    ioport_set_pin_mode(I2C0_CLK, IOPORT_MODE_MUX_A);
    ioport_disable_pin(I2C0_CLK);
	
	/* Configure SPI pins */
	ioport_set_pin_mode(SPI1_MISO, IOPORT_MODE_MUX_A);
	ioport_disable_pin(SPI1_MISO);
	ioport_set_pin_mode(SPI1_MOSI, IOPORT_MODE_MUX_A);
	ioport_disable_pin(SPI1_MOSI);
	ioport_set_pin_mode(SPI1_SPCK, IOPORT_MODE_MUX_A);
	ioport_disable_pin(SPI1_SPCK);
	ioport_set_pin_mode(SPI1_NPCS0, IOPORT_MODE_MUX_A);
	ioport_disable_pin(SPI1_NPCS0);

    /* Configure micro SD Card pins */
    ioport_set_pin_mode(MSD_CMD, IOPORT_MODE_MUX_A);
	ioport_disable_pin(MSD_CMD);
    ioport_set_pin_mode(MSD_CLK, IOPORT_MODE_MUX_A);
	ioport_disable_pin(MSD_CLK);
    ioport_set_pin_mode(MSD_DATA0, IOPORT_MODE_MUX_A);
	ioport_disable_pin(MSD_DATA0);
    ioport_set_pin_mode(MSD_DATA1, IOPORT_MODE_MUX_A);
	ioport_disable_pin(MSD_DATA1);
    ioport_set_pin_mode(MSD_DATA2, IOPORT_MODE_MUX_A);
	ioport_disable_pin(MSD_DATA2);
    ioport_set_pin_mode(MSD_DATA3, IOPORT_MODE_MUX_A);
	ioport_disable_pin(MSD_DATA3);


    /* Init PWM */
    /*pmc_enable_periph_clk(ID_PWM);
    pwm_channel_disable(PWM, MOTOR_LEFT);
    pwm_channel_disable(PWM, MOTOR_RIGHT);
    pwm_channel_disable(PWM, MOTOR_REAR);
    pwm_clock_t clock_setting =
    {
        .ul_clka = PWM_FREQUENCY * PERIOD_VALUE,
        .ul_clkb = 0,
        .ul_mck = sysclk_get_cpu_hz()
    };
    pwm_init(PWM, &clock_setting);*/
}