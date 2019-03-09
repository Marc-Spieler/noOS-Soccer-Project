/**
 * \file
 *
 * \brief Two-Wire Interface (TWI) driver for SAM.
 *
 * Copyright (c) 2011-2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "twi.h"
#include "pdc.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

/**
 * \defgroup sam_drivers_twi_group Two-Wire Interface (TWI)
 *
 * Driver for the TWI (Two-Wire Interface). This driver provides access to the main 
 * features of the TWI controller.
 * The TWI interconnects components on a unique two-wire bus.
 * The TWI is programmable as a master or a slave with sequential or single-byte access.
 * Multiple master capability is supported.
 *
 * \par Usage
 *
 * -# Enable the TWI peripheral clock in the PMC.
 * -# Enable the required TWI PIOs (see pio.h).
 * -# Enable TWI master mode by calling twi_enable_master_mode if it is a master on the I2C bus.
 * -# Configure the TWI in master mode by calling twi_master_init.
 * -# Send data to a slave device on the I2C bus by calling twi_master_write.
 * -# Receive data from a slave device on the I2C bus by calling the twi_master_read.
 * -# Enable TWI slave mode by calling twi_enable_slave_mode if it is a slave on the I2C bus.
 * -# Configure the TWI in slave mode by calling twi_slave_init.
 *
 * @{
 */

/* Low level time limit of I2C Fast Mode. */
#define LOW_LEVEL_TIME_LIMIT 384000
#define I2C_FAST_MODE_SPEED  400000
#define TWI_CLK_DIVIDER      2
#if SAMG55
#define TWI_CLK_CALC_ARGU    3
#else
#define TWI_CLK_CALC_ARGU    4
#endif	
#define TWI_CLK_DIV_MAX      0xFF
#define TWI_CLK_DIV_MIN      7

#define TWI_WP_KEY_VALUE TWI_WPMR_WPKEY_PASSWD

#define MASK_ALL_INTERRUPTS (0xffffffffUL)
#define IER_ERROR_INTERRUPTS (TWI_IER_NACK | TWI_IER_ARBLST | TWI_IER_OVRE)
#define SR_ERROR_INTERRUPTS (TWI_SR_NACK | TWI_SR_ARBLST | TWI_SR_OVRE)
#define TWI_TIMEOUT_COUNTER (0x0000ffffUL)

uint32_t timeout = TWI_TIMEOUT;

static twi_packet_t txPacket;
static twi_packet_t rxPacket;
static uint8_t twiBusy = false;
static void (*lcdTxCallback)(void) = NULL;
static void (*compassTxCallback)(void) = NULL;
static void (*compassRxCallback)(void) = NULL;

/**
 * \brief Enable TWI master mode.
 *
 * \param p_twi Pointer to a TWI instance.
 */
void twi_enable_master_mode(Twi *p_twi)
{
	/* Set Master Disable bit and Slave Disable bit */
	p_twi->TWI_CR = TWI_CR_MSDIS;
	p_twi->TWI_CR = TWI_CR_SVDIS;

	/* Set Master Enable bit */
	p_twi->TWI_CR = TWI_CR_MSEN;
}

/**
 * \brief Disable TWI master mode.
 *
 * \param p_twi Pointer to a TWI instance.
 */
void twi_disable_master_mode(Twi *p_twi)
{
	/* Set Master Disable bit */
	p_twi->TWI_CR = TWI_CR_MSDIS;
}

/**
 * \brief Initialize TWI master mode.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_opt Options for initializing the TWI module (see \ref twi_options_t).
 *
 * \return TWI_SUCCESS if initialization is complete, error code otherwise.
 */
uint32_t twi_master_init(Twi *p_twi, const twi_options_t *p_opt)
{
	uint32_t status = TWI_SUCCESS;

	/* Disable TWI interrupts */
	p_twi->TWI_IDR = ~0UL;

	/* Dummy read in status register */
	p_twi->TWI_SR;

	twi_disable_interrupt(p_twi, MASK_ALL_INTERRUPTS);

	/* Reset TWI peripheral */
	twi_reset(p_twi);

	twi_enable_master_mode(p_twi);

	/* Select the speed */
	if (twi_set_speed(p_twi, p_opt->speed, p_opt->master_clk) == FAIL) {
		/* The desired speed setting is rejected */
		status = TWI_INVALID_ARGUMENT;
	}

	if (p_opt->smbus == 1) {
		p_twi->TWI_CR = TWI_CR_QUICK;
	}
    
    /* Error interrupts are always enabled. */
    twi_enable_interrupt(p_twi, IER_ERROR_INTERRUPTS);

    twiBusy = false;
    
	return status;
}

/**
 * \brief Set the I2C bus speed in conjunction with the clock frequency.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param ul_speed The desired I2C bus speed (in Hz).
 * \param ul_mck Main clock of the device (in Hz).
 *
 * \retval PASS New speed setting is accepted.
 * \retval FAIL New speed setting is rejected.
 */
uint32_t twi_set_speed(Twi *p_twi, uint32_t ul_speed, uint32_t ul_mck)
{
	uint32_t ckdiv = 0;
	uint32_t c_lh_div;
	uint32_t cldiv, chdiv;

	if (ul_speed > I2C_FAST_MODE_SPEED) {
		return FAIL;
	}

	/* Low level time not less than 1.3us of I2C Fast Mode. */
	if (ul_speed > LOW_LEVEL_TIME_LIMIT) {
		/* Low level of time fixed for 1.3us. */
		cldiv = ul_mck / (LOW_LEVEL_TIME_LIMIT * TWI_CLK_DIVIDER) - TWI_CLK_CALC_ARGU;
		chdiv = ul_mck / ((ul_speed + (ul_speed - LOW_LEVEL_TIME_LIMIT)) * TWI_CLK_DIVIDER) - TWI_CLK_CALC_ARGU;
		
		/* cldiv must fit in 8 bits, ckdiv must fit in 3 bits */
		while ((cldiv > TWI_CLK_DIV_MAX) && (ckdiv < TWI_CLK_DIV_MIN)) {
			/* Increase clock divider */
			ckdiv++;
			/* Divide cldiv value */
			cldiv /= TWI_CLK_DIVIDER;
		}
		/* chdiv must fit in 8 bits, ckdiv must fit in 3 bits */
		while ((chdiv > TWI_CLK_DIV_MAX) && (ckdiv < TWI_CLK_DIV_MIN)) {
			/* Increase clock divider */
			ckdiv++;
			/* Divide cldiv value */
			chdiv /= TWI_CLK_DIVIDER;
		}

		/* set clock waveform generator register */
		p_twi->TWI_CWGR =
				TWI_CWGR_CLDIV(cldiv) | TWI_CWGR_CHDIV(chdiv) |
				TWI_CWGR_CKDIV(ckdiv);		
	} else {
		c_lh_div = ul_mck / (ul_speed * TWI_CLK_DIVIDER) - TWI_CLK_CALC_ARGU;

		/* cldiv must fit in 8 bits, ckdiv must fit in 3 bits */
		while ((c_lh_div > TWI_CLK_DIV_MAX) && (ckdiv < TWI_CLK_DIV_MIN)) {
			/* Increase clock divider */
			ckdiv++;
			/* Divide cldiv value */
			c_lh_div /= TWI_CLK_DIVIDER;
		}

		/* set clock waveform generator register */
		p_twi->TWI_CWGR =
				TWI_CWGR_CLDIV(c_lh_div) | TWI_CWGR_CHDIV(c_lh_div) |
				TWI_CWGR_CKDIV(ckdiv);
	}

	return PASS;
}

/**
 * \brief Test if a chip answers a given I2C address.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param uc_slave_addr Address of the remote chip to search for.
 *
 * \return TWI_SUCCESS if a chip was found, error code otherwise.
 */
uint32_t twi_probe(Twi *p_twi, uint8_t uc_slave_addr)
{
	twi_packet_t packet;

	/* Data to send */
	packet.buffer[0] = 0;
	/* Data length */
	packet.length = 1;
	/* Slave chip address */
	packet.chip = (uint32_t) uc_slave_addr;
	/* Internal chip address */
	packet.addr[0] = 0;
	/* Address length */
	packet.addr_length = 0;

	/* Perform a master write access */
	return (twi_master_write(p_twi, &packet));
}


/**
 * \internal
 * \brief Construct the TWI module address register field
 *
 * The TWI module address register is sent out MSB first. And the size controls
 * which byte is the MSB to start with.
 *
 * Please see the device datasheet for details on this.
 */
uint32_t twi_mk_addr(const uint8_t *addr, int len)
{
	uint32_t val;

	if (len == 0)
		return 0;

	val = addr[0];
	if (len > 1) {
		val <<= 8;
		val |= addr[1];
	}
	if (len > 2) {
		val <<= 8;
		val |= addr[2];
	}
	return val;
}

/**
 * \brief Read multiple bytes from a TWI compatible slave device.
 *
 * \note This function will NOT return until all data has been read or error occurs.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_packet Packet information and data (see \ref twi_packet_t).
 *
 * \return TWI_SUCCESS if all bytes were read, error code otherwise.
 */
uint32_t twi_master_read(Twi *p_twi, twi_packet_t *p_packet)
{
	uint32_t status;
	uint32_t cnt = p_packet->length;
	uint8_t *buffer = p_packet->buffer;
	uint8_t stop_sent = 0;
	
	/* Check argument */
	if (cnt == 0) {
		return TWI_INVALID_ARGUMENT;
	}

	/* Set read mode, slave address and 3 internal address byte lengths */
	p_twi->TWI_MMR = 0;
	p_twi->TWI_MMR = TWI_MMR_MREAD | TWI_MMR_DADR(p_packet->chip) |
			((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) &
			TWI_MMR_IADRSZ_Msk);

	/* Set internal address for remote chip */
	p_twi->TWI_IADR = 0;
	p_twi->TWI_IADR = twi_mk_addr(p_packet->addr, p_packet->addr_length);

	/* Send a START condition */
	if (cnt == 1) {
		p_twi->TWI_CR = TWI_CR_START | TWI_CR_STOP;
		stop_sent = 1;
	} else {
		p_twi->TWI_CR = TWI_CR_START;
		stop_sent = 0;
	}

	while (cnt > 0) {
		status = p_twi->TWI_SR;
		if (status & TWI_SR_NACK) {
			return TWI_RECEIVE_NACK;
		}

		if (!timeout--) {
			return TWI_ERROR_TIMEOUT;
		}
				
		/* Last byte ? */
		if (cnt == 1  && !stop_sent) {
			p_twi->TWI_CR = TWI_CR_STOP;
			stop_sent = 1;
		}

		if (!(status & TWI_SR_RXRDY)) {
			continue;
		}
		*buffer++ = p_twi->TWI_RHR;

		cnt--;
		timeout = TWI_TIMEOUT;
	}
  
  timeout = TWI_TIMEOUT;
  
	while (!(p_twi->TWI_SR & TWI_SR_TXCOMP)) {
    if (!timeout--) {
      return TWI_ERROR_TIMEOUT;
    }
	}

	p_twi->TWI_SR;

	return TWI_SUCCESS;
}

/**
 * \brief Write multiple bytes to a TWI compatible slave device.
 *
 * \note This function will NOT return until all data has been written or error occurred.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_packet Packet information and data (see \ref twi_packet_t).
 *
 * \return TWI_SUCCESS if all bytes were written, error code otherwise.
 */
uint32_t twi_master_write(Twi *p_twi, twi_packet_t *p_packet)
{
    uint32_t status;
    uint32_t cnt = p_packet->length;
    uint8_t *buffer = p_packet->buffer;

    /* Check argument */
    if (cnt == 0) {
        return TWI_INVALID_ARGUMENT;
    }

    /* Set write mode, slave address and 3 internal address byte lengths */
    p_twi->TWI_MMR = 0;
    p_twi->TWI_MMR = TWI_MMR_DADR(p_packet->chip) |
        ((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) &
        TWI_MMR_IADRSZ_Msk);

    /* Set internal address for remote chip */
    p_twi->TWI_IADR = 0;
    p_twi->TWI_IADR = twi_mk_addr(p_packet->addr, p_packet->addr_length);

    /* Send all bytes */
    while (cnt > 0) {
        status = p_twi->TWI_SR;
        if (status & TWI_SR_NACK) {
            return TWI_RECEIVE_NACK;
        }
    
        timeout = TWI_TIMEOUT;
    
        if (!(status & TWI_SR_TXRDY)) {
            if (!timeout--) {
                return TWI_ERROR_TIMEOUT;
            }
      
            continue;
        }
        p_twi->TWI_THR = *buffer++;
  
        timeout = 0;
    
        cnt--;
    }
  
    timeout = TWI_TIMEOUT;
  
    while (1) {
        status = p_twi->TWI_SR;
        if (status & TWI_SR_NACK) {
            return TWI_RECEIVE_NACK;
        }

        if (status & TWI_SR_TXRDY) {
            break;
        }
    
        if (!timeout--) {
            return TWI_ERROR_TIMEOUT;
        }
    }

    p_twi->TWI_CR = TWI_CR_STOP;
  
    timeout = TWI_TIMEOUT;
  
    while (!(p_twi->TWI_SR & TWI_SR_TXCOMP)) {
        if (!timeout--) {
            return TWI_ERROR_TIMEOUT;
        }
    }

    return TWI_SUCCESS;
}

#if 0
/**
 * \brief Read multiple bytes from a TWI compatible slave device.
 *
 * \note This function will NOT return until all data has been read or error occurs.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_packet Packet information and data (see \ref twi_packet_t).
 *
 * \return TWI_SUCCESS if all bytes were read, error code otherwise.
 */
uint32_t twi_pdc_master_read(Twi *p_twi, twi_packet_t *p_packet)
{
    pdc_packet_t rxPacket;

    // Check argument
    if(p_packet->length == 0)
    {
        return TWI_INVALID_ARGUMENT;
    }

    // Ensure Rx is already empty
    twi_read_byte(TWI0);

    rxPacket.ul_addr = (uint32_t)p_packet->buffer;
    rxPacket.ul_size = p_packet->length - 1;
    pdc_rx_init(PDC_TWI0, &rxPacket, NULL);

    // Set read mode, slave address and 3 internal address byte lengths
    p_twi->TWI_MMR = 0;
    p_twi->TWI_MMR = TWI_MMR_MREAD | TWI_MMR_DADR(p_packet->chip) | ((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) & TWI_MMR_IADRSZ_Msk);

    // Set internal address for remote chip
    p_twi->TWI_IADR = 0;
    p_twi->TWI_IADR = twi_mk_addr(p_packet->addr, p_packet->addr_length);

    pdc_enable_transfer(PDC_TWI0, PERIPH_PTCR_RXTEN);
    TWI0->TWI_CR = TWI_CR_START;

    twi_enable_interrupt(p_twi, TWI_IER_ENDRX);
    NVIC_ClearPendingIRQ(TWI0_IRQn);
    NVIC_EnableIRQ(TWI0_IRQn);

    return TWI_SUCCESS;
}

/**
 * \brief Write multiple bytes to a TWI compatible slave device.
 *
 * \note This function will NOT return until all data has been written or error occurred.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_packet Packet information and data (see \ref twi_packet_t).
 *
 * \return TWI_SUCCESS if all bytes were written, error code otherwise.
 */
uint32_t twi_pdc_master_write(Twi *p_twi, twi_packet_t *p_packet)
{
    pdc_packet_t txPacket;

    // Check argument
    if(p_packet->length == 0)
    {
        return TWI_INVALID_ARGUMENT;
    }

    txPacket.ul_addr = (uint32_t)p_packet->buffer;
    txPacket.ul_size = p_packet->length;
    pdc_tx_init(PDC_TWI0, &txPacket, NULL);

    // Set write mode, slave address and 3 internal address byte lengths
    p_twi->TWI_MMR = 0;
    p_twi->TWI_MMR = TWI_MMR_DADR(p_packet->chip) | ((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) & TWI_MMR_IADRSZ_Msk);

    // Set internal address for remote chip
    p_twi->TWI_IADR = 0;
    p_twi->TWI_IADR = twi_mk_addr(p_packet->addr, p_packet->addr_length);

    pdc_enable_transfer(PDC_TWI0, PERIPH_PTCR_TXTEN);

    twi_enable_interrupt(p_twi, TWI_IER_ENDTX);
    NVIC_ClearPendingIRQ(TWI0_IRQn);
    NVIC_EnableIRQ(TWI0_IRQn);

    return TWI_SUCCESS;
}

/**
 * \brief Callback function for TWI receive.
 *
 * \param callback Pointer to callback function.
 */
void twi_set_compass_rx_callback(void (*callback)(uint8_t))
{
    compassRxCallback = callback;
}

/**
 * \brief Callback function for TWI transmit.
 *
 * \param callback Pointer to callback function.
 */
void twi_set_compass_tx_callback(void (*callback)(void))
{
    compassTxCallback = callback;
}

/**
 * \brief Callback function for TWI transmit.
 *
 * \param callback Pointer to callback function.
 */
void twi_set_lcd_tx_callback(void (*callback)(void))
{
    lcdTxCallback = callback;
}

/**
 * \brief TWI0 Interrupt handler.
 *
 */
void TWI0_Handler(void)
{
    uint32_t status = twi_get_interrupt_status(TWI0) & twi_get_interrupt_mask(TWI0);
    uint8_t lastRxByte;

    // End of PDC transfer -> switch PDC off
    if(status & TWI_SR_ENDTX)
    {
        pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS);
        twi_disable_interrupt(TWI0, TWI_IER_ENDTX);

        twi_enable_interrupt(TWI0, TWI_IER_TXCOMP);
        TWI0->TWI_CR = TWI_CR_STOP;

        status = twi_get_interrupt_status(TWI0);
    }

    // End of transfer -> switch TWI off
    if(status & TWI_SR_TXCOMP)
    {
        twi_disable_interrupt(TWI0, TWI_IER_TXCOMP);

        // If defined, call the connected function.
        if(compassTxCallback != NULL)
        {
            compassTxCallback();
        }

        // If defined, call the connected function.
        if(lcdTxCallback != NULL)
        {
          lcdTxCallback();
        }
    }

    // End of PDC transfer -> switch PDC off
    if(status & TWI_SR_ENDRX)
    {
        pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_RXTDIS);
        twi_disable_interrupt(TWI0, TWI_IER_ENDRX);

        twi_enable_interrupt(TWI0, TWI_IER_RXRDY);
        TWI0->TWI_CR = TWI_CR_STOP;

        status = twi_get_interrupt_status(TWI0);
    }        

    // End of transfer -> switch TWI off
    if(status & TWI_SR_RXRDY)
    {
        twi_disable_interrupt(TWI0, TWI_IER_RXRDY);

        // Read last data
        lastRxByte = TWI0->TWI_RHR;

        // If defined, call the connected function.
        if(compassRxCallback != NULL)
        {
            compassRxCallback(lastRxByte);
        }
    }
}
#endif
#if 0
/**
 * \brief Read multiple bytes from a TWI compatible slave device.
 *
 * \note This function will NOT return until all data has been read or error occurs.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_packet Packet information and data (see \ref twi_packet_t).
 *
 * \return TWI_SUCCESS if all bytes were read, error code otherwise.
 */
uint32_t twi_pdc_master_read(Twi *p_twi, twi_packet_t *p_packet)
{
    pdc_packet_t rxPacket;

    // Check argument
    if(p_packet->length == 0)
    {
        return TWI_INVALID_ARGUMENT;
    }

    // Ensure Rx is already empty
    twi_read_byte(TWI0);

    rxPacket.ul_addr = (uint32_t)p_packet->buffer;
    rxPacket.ul_size = p_packet->length - 2;
    pdc_rx_init(PDC_TWI0, &rxPacket, NULL);

    // Set read mode, slave address and 3 internal address byte lengths
    p_twi->TWI_MMR = 0;
    p_twi->TWI_MMR = TWI_MMR_MREAD | TWI_MMR_DADR(p_packet->chip) | ((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) & TWI_MMR_IADRSZ_Msk);

    // Set internal address for remote chip
    p_twi->TWI_IADR = 0;
    p_twi->TWI_IADR = twi_mk_addr(p_packet->addr, p_packet->addr_length);

    pdc_enable_transfer(PDC_TWI0, PERIPH_PTCR_RXTEN);
    TWI0->TWI_CR = TWI_CR_START;

    twi_enable_interrupt(p_twi, TWI_IER_ENDRX);
    NVIC_ClearPendingIRQ(TWI0_IRQn);
    NVIC_EnableIRQ(TWI0_IRQn);

    return TWI_SUCCESS;
}

/**
 * \brief Write multiple bytes to a TWI compatible slave device.
 *
 * \note This function will NOT return until all data has been written or error occurred.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_packet Packet information and data (see \ref twi_packet_t).
 *
 * \return TWI_SUCCESS if all bytes were written, error code otherwise.
 */
uint32_t twi_pdc_master_write(Twi *p_twi, twi_packet_t *p_packet)
{
    pdc_packet_t txPacket;

    // Check argument
    if(p_packet->length == 0)
    {
        return TWI_INVALID_ARGUMENT;
    }

    txPacket.ul_addr = (uint32_t)p_packet->buffer;
    txPacket.ul_size = p_packet->length;    // - 1;
    pdc_tx_init(PDC_TWI0, &txPacket, NULL);

    // Set write mode, slave address and 3 internal address byte lengths
    p_twi->TWI_MMR = 0;
    p_twi->TWI_MMR = TWI_MMR_DADR(p_packet->chip) | ((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) & TWI_MMR_IADRSZ_Msk);

    // Set internal address for remote chip
    p_twi->TWI_IADR = 0;
    p_twi->TWI_IADR = twi_mk_addr(p_packet->addr, p_packet->addr_length);

    pdc_enable_transfer(PDC_TWI0, PERIPH_PTCR_TXTEN);

    twi_enable_interrupt(p_twi, TWI_IER_ENDTX);
    NVIC_ClearPendingIRQ(TWI0_IRQn);
    NVIC_EnableIRQ(TWI0_IRQn);

    return TWI_SUCCESS;
}

/**
 * \brief Callback function for TWI receive.
 *
 * \param callback Pointer to callback function.
 */
void twi_set_rx_callback(void (*callback)(uint8_t))
{
    rxCallback = callback;
}

/**
 * \brief Callback function for TWI transmit.
 *
 * \param callback Pointer to callback function.
 */
void twi_set_tx_callback(void (*callback)(void))
{
    txCallback = callback;
}

#define SR_ERROR_INTERRUPTS (TWI_SR_NACK | TWI_SR_ARBLST | TWI_SR_OVRE)
#define TWI_TIMEOUT_COUNTER (0x0000ffffUL)

/**
 * \brief TWI0 Interrupt handler.
 *
 */
void TWI0_Handler(void)
{
    uint32_t twi_status = twi_get_interrupt_status(TWI0) & twi_get_interrupt_mask(TWI0);
    uint8_t lastRxByte;
	bool transfer_timeout = false;
	uint8_t status;
	uint32_t timeout_counter = 0;

    // End of PDC transfer -> switch PDC off
    if(twi_status & TWI_SR_ENDTX)
    {
        pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS);
        twi_disable_interrupt(TWI0, TWI_IER_ENDTX);

		/* Wait for TX ready flag */
		while (1)
        {
			status = TWI0->TWI_SR;
			if (status & TWI_SR_TXRDY)
            {
				break;
			}
			/* Check timeout condition. */
			if (++timeout_counter >= TWI_TIMEOUT_COUNTER)
            {
        		transfer_timeout = true;
				break;
			}
		}
		/* Complete the transfer - stop and last byte */
		TWI0->TWI_CR = TWI_CR_STOP;
		TWI0->TWI_THR = 0x08;   //twis[twi_index].buffer[twis[twi_index].length-1];

		/* Wait for TX complete flag */
		while (1)
        {
			status = TWI0->TWI_SR;
			if (status & TWI_SR_TXCOMP)
            {
				break;
			}
			/* Check timeout condition. */
			if (++timeout_counter >= TWI_TIMEOUT_COUNTER)
            {
				transfer_timeout = true;
				break;
			}
		}

        // If defined, call the connected function.
        if(txCallback != NULL)
        {
            txCallback();
        }
    }

    // End of PDC transfer -> switch PDC off
    if(twi_status & TWI_SR_ENDRX)
    {
        pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_RXTDIS);
        twi_disable_interrupt(TWI0, TWI_IER_ENDRX);

		/* Wait for RX ready flag */
		while (1) {
			status = TWI0->TWI_SR;
			if (status & TWI_SR_RXRDY)
            {
				break;
			}
			/* Check timeout condition. */
			if (++timeout_counter >= TWI_TIMEOUT_COUNTER)
            {
				break;
			}
		}
		/* Complete the transfer. */
		TWI0->TWI_CR = TWI_CR_STOP;
		/* Read second last data */
        lastRxByte = TWI0->TWI_RHR;

		/* Wait for RX ready flag */
		while (1)
        {
			status = TWI0->TWI_SR;
			if (status & TWI_SR_RXRDY)
            {
				break;
			}
			/* Check timeout condition. */
			if (++timeout_counter >= TWI_TIMEOUT_COUNTER)
            {
				break;
			}
		}

		if (!(timeout_counter >= TWI_TIMEOUT_COUNTER))
        {
			/* Read last data */
			lastRxByte = TWI0->TWI_RHR;
			timeout_counter = 0;
			/* Wait for TX complete flag before releasing semaphore */
			while (1)
            {
				status = TWI0->TWI_SR;
				if (status & TWI_SR_TXCOMP)
                {
					break;
				}
				/* Check timeout condition. */
				if (++timeout_counter >= TWI_TIMEOUT_COUNTER)
                {
					transfer_timeout = true;
					break;
				}
			}
		}

        // If defined, call the connected function.
        if(rxCallback != NULL)
        {
            rxCallback(lastRxByte);
        }
    }        

	if (((twi_status & SR_ERROR_INTERRUPTS) != 0) || (transfer_timeout == true))
    {
		/* An error occurred in either a transmission or reception.  Abort.
		Stop the transmission, disable interrupts used by the peripheral, and
		ensure the peripheral access mutex is made available to tasks.  As this
		peripheral is half duplex, only the Tx peripheral access mutex exits.*/

		/* Stop the PDC */
		pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS | PERIPH_PTCR_RXTDIS);

		if (!(twi_status & TWI_SR_NACK))
        {
			/* Do not send stop if NACK received. Handled by hardware */
			TWI0->TWI_CR = TWI_CR_STOP;
		}
		twi_disable_interrupt(TWI0, TWI_IDR_ENDTX);
		twi_disable_interrupt(TWI0, TWI_IDR_ENDRX);
	}
}
#endif
#if 0
uint8_t twi_is_busy(void)
{
    return twiBusy;
}

twi_packet_t *twi_get_tx_packet(void)
{
    return &txPacket;
}

twi_packet_t *twi_get_rx_packet(void)
{
    return &rxPacket;
}

/**
 * \brief Read multiple bytes from a TWI compatible slave device.
 *
 * \note This function will NOT return until all data has been read or error occurs.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_packet Packet information and data (see \ref twi_packet_t).
 *
 * \return TWI_SUCCESS if all bytes were read, error code otherwise.
 */
uint32_t twi_pdc_master_read(Twi *p_twi, twi_packet_t *p_packet)
{
    pdc_packet_t rxPdcPacket;

    // Check argument
    if(p_packet->length == 0)
    {
        return TWI_INVALID_ARGUMENT;
    }
    
    twiBusy = true;

    // Ensure Rx is already empty
    twi_read_byte(TWI0);

    rxPdcPacket.ul_addr = (uint32_t)p_packet->buffer;
    rxPdcPacket.ul_size = p_packet->length - 1;
    pdc_rx_init(PDC_TWI0, &rxPdcPacket, NULL);

    // Set read mode, slave address and 3 internal address byte lengths
    p_twi->TWI_MMR = 0;
    p_twi->TWI_MMR = TWI_MMR_MREAD | TWI_MMR_DADR(p_packet->chip) | ((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) & TWI_MMR_IADRSZ_Msk);

    // Set internal address for remote chip
    p_twi->TWI_IADR = 0;
    p_twi->TWI_IADR = twi_mk_addr(p_packet->addr, p_packet->addr_length);

    pdc_enable_transfer(PDC_TWI0, PERIPH_PTCR_RXTEN);
    TWI0->TWI_CR = TWI_CR_START;

    twi_enable_interrupt(p_twi, TWI_IER_ENDRX);
    NVIC_ClearPendingIRQ(TWI0_IRQn);
    NVIC_EnableIRQ(TWI0_IRQn);

    return TWI_SUCCESS;
}

/**
 * \brief Write multiple bytes to a TWI compatible slave device.
 *
 * \note This function will NOT return until all data has been written or error occurred.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_packet Packet information and data (see \ref twi_packet_t).
 *
 * \return TWI_SUCCESS if all bytes were written, error code otherwise.
 */
uint32_t twi_pdc_master_write(Twi *p_twi, twi_packet_t *p_packet)
{
    pdc_packet_t txPdcPacket;

    // Check argument
    if(p_packet->length == 0)
    {
        return TWI_INVALID_ARGUMENT;
    }

    twiBusy = true;

    txPdcPacket.ul_addr = (uint32_t)p_packet->buffer;
    txPdcPacket.ul_size = p_packet->length - 1;
    pdc_tx_init(PDC_TWI0, &txPdcPacket, NULL);

    // Set write mode, slave address and 3 internal address byte lengths
    p_twi->TWI_MMR = 0;
    p_twi->TWI_MMR = TWI_MMR_DADR(p_packet->chip) | ((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) & TWI_MMR_IADRSZ_Msk);

    // Set internal address for remote chip
    p_twi->TWI_IADR = 0;
    p_twi->TWI_IADR = twi_mk_addr(p_packet->addr, p_packet->addr_length);

    pdc_enable_transfer(PDC_TWI0, PERIPH_PTCR_TXTEN);

    twi_enable_interrupt(p_twi, TWI_IER_ENDTX);
    NVIC_ClearPendingIRQ(TWI0_IRQn);
    NVIC_EnableIRQ(TWI0_IRQn);

    return TWI_SUCCESS;
}

/**
 * \brief Callback function for TWI receive.
 *
 * \param callback Pointer to callback function.
 */
void twi_set_rx_callback(void (*callback)(void))
{
    rxCallback = callback;
}

/**
 * \brief Callback function for TWI transmit.
 *
 * \param callback Pointer to callback function.
 */
void twi_set_tx_callback(void (*callback)(void))
{
    txCallback = callback;
}

#define SR_ERROR_INTERRUPTS (TWI_SR_NACK | TWI_SR_ARBLST | TWI_SR_OVRE)
#define TWI_TIMEOUT_COUNTER (0x0000ffffUL)

/**
 * \brief TWI0 Interrupt handler.
 *
 */
void TWI0_Handler(void)
{
    uint32_t twi_status = twi_get_interrupt_status(TWI0) & twi_get_interrupt_mask(TWI0);

    // End of PDC transfer -> switch PDC off and wait ready flag
    if(twi_status & TWI_SR_ENDTX)
    {
        pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS);
        twi_disable_interrupt(TWI0, TWI_IER_ENDTX);
        twi_enable_interrupt(TWI0, TWI_IER_TXRDY);
        twi_status |= twi_get_interrupt_status(TWI0);
    }
    
    // Received ready flag -> send last byte
    if(twi_status & TWI_SR_TXRDY)
    {
        twi_disable_interrupt(TWI0, TWI_IER_TXRDY);

		// Complete the transfer - stop and last byte 
		TWI0->TWI_CR = TWI_CR_STOP;
		TWI0->TWI_THR = txPacket.buffer[txPacket.length-1];

        twi_enable_interrupt(TWI0, TWI_IER_TXCOMP);
        twi_status |= twi_get_interrupt_status(TWI0);
    }

    // End of transfer -> switch TWI off
    if(twi_status & TWI_SR_TXCOMP)
    {
        twi_disable_interrupt(TWI0, TWI_IER_TXCOMP);

        // If defined, call the connected function.
        if(txCallback != NULL)
        {
            txCallback();
        }

        twiBusy = false;
    }

    // End of PDC transfer -> switch PDC off
    if(twi_status & TWI_SR_ENDRX)
    {
        pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_RXTDIS);
        twi_disable_interrupt(TWI0, TWI_IER_ENDRX);

        twi_enable_interrupt(TWI0, TWI_IER_RXRDY);
        TWI0->TWI_CR = TWI_CR_STOP;

        twi_status |= twi_get_interrupt_status(TWI0);
    }

    // End of transfer -> switch TWI off
    if(twi_status & TWI_SR_RXRDY)
    {
        twi_disable_interrupt(TWI0, TWI_IER_RXRDY);

        // Read last data
        rxPacket.buffer[rxPacket.length-1] = TWI0->TWI_RHR;

        // If defined, call the connected function.
        if(rxCallback != NULL)
        {
            rxCallback();
        }

        twiBusy = false;
    }

	// An error occurred in either a transmission or reception.
    // Abort, stop the transmission and disable interrupts.
	if(twi_status & SR_ERROR_INTERRUPTS)
    {
		// Stop the PDC
		pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS | PERIPH_PTCR_RXTDIS);

		if(!(twi_status & TWI_SR_NACK))
        {
			// Do not send stop if NACK received. Handled by hardware
			TWI0->TWI_CR = TWI_CR_STOP;
		}
		twi_disable_interrupt(TWI0, TWI_IDR_ENDTX);
		twi_disable_interrupt(TWI0, TWI_IDR_ENDRX);

        twiBusy = false;
	}
}
#endif
#if 1
uint8_t twi_is_busy(void)
{
    return twiBusy;
}

twi_packet_t *twi_get_tx_packet(void)
{
    return &txPacket;
}

twi_packet_t *twi_get_rx_packet(void)
{
    return &rxPacket;
}

/**
 * \brief Read multiple bytes from a TWI compatible slave device.
 *
 * \note This function will NOT return until all data has been read or error occurs.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_packet Packet information and data (see \ref twi_packet_t).
 *
 * \return TWI_SUCCESS if all bytes were read, error code otherwise.
 */
uint32_t twi_pdc_master_read(Twi *p_twi, twi_packet_t *p_packet)
{
    pdc_packet_t rxPdcPacket;

    // Check argument
    if(p_packet->length == 0)
    {
        return TWI_INVALID_ARGUMENT;
    }
    
    twiBusy = true;

    // Ensure Rx is already empty
    twi_read_byte(TWI0);

    // Set read mode, slave address and 3 internal address byte lengths
    p_twi->TWI_MMR = 0;
    p_twi->TWI_MMR = TWI_MMR_MREAD | TWI_MMR_DADR(p_packet->chip) | ((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) & TWI_MMR_IADRSZ_Msk);

    // Set internal address for remote chip
    p_twi->TWI_IADR = 0;
    p_twi->TWI_IADR = twi_mk_addr(p_packet->addr, p_packet->addr_length);
#if 0
    if(p_packet->length <= 2)
    {
        /* Do not handle errors for short packets in interrupt handler */
        twi_disable_interrupt(TWI0, IER_ERROR_INTERRUPTS);

        /* Cannot use PDC transfer, use normal transfer */
        uint8_t stop_sent = 0;
        uint32_t cnt = p_packet->length;
        uint32_t status;
        uint8_t *buffer = p_packet->buffer;
        uint32_t timeout_counter = 0;

        /* Start the transfer. */
        if(cnt == 1)
        {
            TWI0->TWI_CR = TWI_CR_START | TWI_CR_STOP;
            stop_sent = 1;
        }
        else
        {
            TWI0->TWI_CR = TWI_CR_START;
        }

        while(cnt > 0)
        {
            status = TWI0->TWI_SR;
            if(status & TWI_SR_NACK)
            {
                /* Re-enable interrupts */
                twi_enable_interrupt(TWI0, IER_ERROR_INTERRUPTS);
                return ERR_BUSY;
            }
            /* Last byte ? */
            if(cnt == 1 && !stop_sent)
            {
                TWI0->TWI_CR = TWI_CR_STOP;
                stop_sent = 1;
            }
            if(!(status & TWI_SR_RXRDY))
            {
                if(++timeout_counter >= TWI_TIMEOUT_COUNTER)
                {
                    return_value = ERR_TIMEOUT;
                    break;
                }
                continue;
            }
            *buffer++ = >TWI0->TWI_RHR;
            cnt--;
            timeout_counter = 0;
        }

        timeout_counter = 0;
        /* Wait for stop to be sent */
        while(!(TWI0->TWI_SR & TWI_SR_TXCOMP))
        {
            /* Check timeout condition. */
            if(++timeout_counter >= TWI_TIMEOUT_COUNTER)
            {
                return_value = ERR_TIMEOUT;
                break;
            }
        }
        /* Re-enable interrupts */
        twi_enable_interrupt(TWI0, IER_ERROR_INTERRUPTS);
    }
    else
    {
#endif
        rxPdcPacket.ul_addr = (uint32_t)p_packet->buffer;
        rxPdcPacket.ul_size = p_packet->length - 1;
        pdc_rx_init(PDC_TWI0, &rxPdcPacket, NULL);

        pdc_enable_transfer(PDC_TWI0, PERIPH_PTCR_RXTEN);
        TWI0->TWI_CR = TWI_CR_START;

        twi_enable_interrupt(p_twi, TWI_IER_ENDRX);
        NVIC_ClearPendingIRQ(TWI0_IRQn);
        NVIC_EnableIRQ(TWI0_IRQn);
//    }

    return TWI_SUCCESS;
}

/**
 * \brief Write multiple bytes to a TWI compatible slave device.
 *
 * \note This function will NOT return until all data has been written or error occurred.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_packet Packet information and data (see \ref twi_packet_t).
 *
 * \return TWI_SUCCESS if all bytes were written, error code otherwise.
 */
uint32_t twi_pdc_master_write(Twi *p_twi, twi_packet_t *p_packet)
{
    pdc_packet_t txPdcPacket;

    // Check argument
    if(p_packet->length == 0)
    {
        return TWI_INVALID_ARGUMENT;
    }

    twiBusy = true;

    txPdcPacket.ul_addr = (uint32_t)p_packet->buffer;
    txPdcPacket.ul_size = p_packet->length - 1;
    pdc_tx_init(PDC_TWI0, &txPdcPacket, NULL);

    // Set write mode, slave address and 3 internal address byte lengths
    p_twi->TWI_MMR = 0;
    p_twi->TWI_MMR = TWI_MMR_DADR(p_packet->chip) | ((p_packet->addr_length << TWI_MMR_IADRSZ_Pos) & TWI_MMR_IADRSZ_Msk);

    // Set internal address for remote chip
    p_twi->TWI_IADR = 0;
    p_twi->TWI_IADR = twi_mk_addr(p_packet->addr, p_packet->addr_length);

    pdc_enable_transfer(PDC_TWI0, PERIPH_PTCR_TXTEN);

    twi_enable_interrupt(p_twi, TWI_IER_ENDTX);
    NVIC_ClearPendingIRQ(TWI0_IRQn);
    NVIC_EnableIRQ(TWI0_IRQn);

    return TWI_SUCCESS;
}

/**
 * \brief Callback function for TWI receive.
 *
 * \param callback Pointer to callback function.
 */
void twi_set_compass_rx_callback(void (*callback)(void))
{
    compassRxCallback = callback;
}

/**
 * \brief Callback function for TWI transmit.
 *
 * \param callback Pointer to callback function.
 */
void twi_set_compass_tx_callback(void (*callback)(void))
{
    compassTxCallback = callback;
}

/**
 * \brief Callback function for TWI transmit.
 *
 * \param callback Pointer to callback function.
 */
void twi_set_lcd_tx_callback(void (*callback)(void))
{
    lcdTxCallback = callback;
}

/**
 * \brief TWI0 Interrupt handler.
 *
 */
void TWI0_Handler(void)
{
    uint32_t twi_status = twi_get_interrupt_status(TWI0) & twi_get_interrupt_mask(TWI0);

    // End of PDC transfer -> switch PDC off and wait ready flag
    if(twi_status & TWI_SR_ENDTX)
    {
        pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS);
        twi_disable_interrupt(TWI0, TWI_IER_ENDTX);
        twi_enable_interrupt(TWI0, TWI_IER_TXRDY);
        twi_status |= twi_get_interrupt_status(TWI0);
    }
    
    // Received ready flag -> send last byte
    if(twi_status & TWI_SR_TXRDY)
    {
        twi_disable_interrupt(TWI0, TWI_IER_TXRDY);

		// Complete the transfer - stop and last byte 
		TWI0->TWI_CR = TWI_CR_STOP;
		TWI0->TWI_THR = txPacket.buffer[txPacket.length-1];

        twi_enable_interrupt(TWI0, TWI_IER_TXCOMP);
        twi_status |= twi_get_interrupt_status(TWI0);
    }

    // End of transfer -> switch TWI off
    if(twi_status & TWI_SR_TXCOMP)
    {
        twi_disable_interrupt(TWI0, TWI_IER_TXCOMP);

        // If defined, call the connected function.
        if(compassTxCallback != NULL)
        {
            compassTxCallback();
        }

        // If defined, call the connected function.
        if(lcdTxCallback != NULL)
        {
          lcdTxCallback();
        }

        twiBusy = false;
    }

    // End of PDC transfer -> switch PDC off
    if(twi_status & TWI_SR_ENDRX)
    {
        pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_RXTDIS);
        twi_disable_interrupt(TWI0, TWI_IER_ENDRX);

        twi_enable_interrupt(TWI0, TWI_IER_RXRDY);
        TWI0->TWI_CR = TWI_CR_STOP;

        twi_status |= twi_get_interrupt_status(TWI0);
    }

    // End of transfer -> switch TWI off
    if(twi_status & TWI_SR_RXRDY)
    {
        twi_disable_interrupt(TWI0, TWI_IER_RXRDY);

        // Read last data
        rxPacket.buffer[rxPacket.length-1] = TWI0->TWI_RHR;

        // If defined, call the connected function.
        if(compassRxCallback != NULL)
        {
            compassRxCallback();
        }

        twiBusy = false;
    }

	// An error occurred in either a transmission or reception.
    // Abort, stop the transmission and disable interrupts.
	if(twi_status & SR_ERROR_INTERRUPTS)
    {
		// Stop the PDC
		pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS | PERIPH_PTCR_RXTDIS);

		if(!(twi_status & TWI_SR_NACK))
        {
			// Do not send stop if NACK received. Handled by hardware
			TWI0->TWI_CR = TWI_CR_STOP;
		}
		twi_disable_interrupt(TWI0, TWI_IDR_ENDTX);
		twi_disable_interrupt(TWI0, TWI_IDR_ENDRX);

        twiBusy = false;
	}
}
#endif
/**
 * \brief Enable TWI interrupts.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param ul_sources Interrupts to be enabled.
 */
void twi_enable_interrupt(Twi *p_twi, uint32_t ul_sources)
{
	/* Enable the specified interrupts */
	p_twi->TWI_IER = ul_sources;
}

/**
 * \brief Disable TWI interrupts.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param ul_sources Interrupts to be disabled.
 */
void twi_disable_interrupt(Twi *p_twi, uint32_t ul_sources)
{
	/* Disable the specified interrupts */
	p_twi->TWI_IDR = ul_sources;
	/* Dummy read */
	p_twi->TWI_SR;
}

/**
 * \brief Get TWI interrupt status.
 *
 * \param p_twi Pointer to a TWI instance.
 *
 * \retval TWI interrupt status.
 */
uint32_t twi_get_interrupt_status(Twi *p_twi)
{
	return p_twi->TWI_SR;
}

/**
 * \brief Read TWI interrupt mask.
 *
 * \param p_twi Pointer to a TWI instance.
 *
 * \return The interrupt mask value.
 */
uint32_t twi_get_interrupt_mask(Twi *p_twi)
{
	return p_twi->TWI_IMR;
}

/**
 * \brief Reads a byte from the TWI bus.
 *
 * \param p_twi Pointer to a TWI instance.
 *
 * \return The byte read.
 */
uint8_t twi_read_byte(Twi *p_twi)
{
	return p_twi->TWI_RHR;
}

/**
 * \brief Sends a byte of data to one of the TWI slaves on the bus.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param byte The byte to send.
 */
void twi_write_byte(Twi *p_twi, uint8_t uc_byte)
{
	p_twi->TWI_THR = uc_byte;
}

/**
 * \brief Enable TWI slave mode.
 *
 * \param p_twi Pointer to a TWI instance.
 */
void twi_enable_slave_mode(Twi *p_twi)
{
	/* Set Master Disable bit and Slave Disable bit */
	p_twi->TWI_CR = TWI_CR_MSDIS;
	p_twi->TWI_CR = TWI_CR_SVDIS;

	/* Set Slave Enable bit */
	p_twi->TWI_CR = TWI_CR_SVEN;
}

/**
 * \brief Disable TWI slave mode.
 *
 * \param p_twi Pointer to a TWI instance.
 */
void twi_disable_slave_mode(Twi *p_twi)
{
	/* Set Slave Disable bit */
	p_twi->TWI_CR = TWI_CR_SVDIS;
}

/**
 * \brief Initialize TWI slave mode.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param ul_device_addr Device address of the SAM slave device on the I2C bus.
 */
void twi_slave_init(Twi *p_twi, uint32_t ul_device_addr)
{
	/* Disable TWI interrupts */
	p_twi->TWI_IDR = ~0UL;
	p_twi->TWI_SR;

	/* Reset TWI */
	twi_reset(p_twi);

	/* Set slave address in slave mode */
	p_twi->TWI_SMR = TWI_SMR_SADR(ul_device_addr);

	/* Enable slave mode */
	twi_enable_slave_mode(p_twi);
}

/**
 * \brief Set TWI slave address.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param ul_device_addr Device address of the SAM slave device on the I2C bus.
 */
void twi_set_slave_addr(Twi *p_twi, uint32_t ul_device_addr)
{
	/* Set slave address */
	p_twi->TWI_SMR = TWI_SMR_SADR(ul_device_addr);
}

/**
 * \brief Read data from master.
 *
 * \note This function will NOT return until master sends a STOP condition.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_data Pointer to the data buffer where data received will be stored.
 *
 * \return Number of bytes read.
 */
uint32_t twi_slave_read(Twi *p_twi, uint8_t *p_data)
{
	uint32_t status, cnt = 0;
  
  timeout = TWI_TIMEOUT;
  
	do {
		status = p_twi->TWI_SR;
		if (status & TWI_SR_SVACC) {
			if (!(status & TWI_SR_GACC) &&
				((status & (TWI_SR_SVREAD | TWI_SR_RXRDY))
				 == (TWI_SR_SVREAD | TWI_SR_RXRDY))) {
				*p_data++ = (uint8_t) p_twi->TWI_RHR;
				cnt++;
			}
		} else if ((status & (TWI_SR_EOSACC | TWI_SR_TXCOMP))
					== (TWI_SR_EOSACC | TWI_SR_TXCOMP)) {
			break;
		}
    
    if (!timeout--) {
      return TWI_ERROR_TIMEOUT;
    }
    
	} while (1);

	return cnt;
}

/**
 * \brief Write data to TWI bus.
 *
 * \note This function will NOT return until master sends a STOP condition.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_data Pointer to the data buffer to be sent.
 *
 * \return Number of bytes written.
 */
uint32_t twi_slave_write(Twi *p_twi, uint8_t *p_data)
{
	uint32_t status, cnt = 0;
  
  timeout = TWI_TIMEOUT;
  
	do {
		status = p_twi->TWI_SR;
		if (status & TWI_SR_SVACC) {
			if (!(status & (TWI_SR_GACC | TWI_SR_SVREAD)) &&
				(status & TWI_SR_TXRDY)) {
				p_twi->TWI_THR = *p_data++;
				cnt++;
			}
		} else if ((status & (TWI_SR_EOSACC | TWI_SR_TXCOMP))
					== (TWI_SR_EOSACC | TWI_SR_TXCOMP)) {
			break;
		}
    
    if (!timeout--) {
      return TWI_ERROR_TIMEOUT;
    }
	} while (1);

	return cnt;
}

/**
 * \brief Reset TWI.
 *
 * \param p_twi Pointer to a TWI instance.
 */
void twi_reset(Twi *p_twi)
{
	/* Set SWRST bit to reset TWI peripheral */
	p_twi->TWI_CR = TWI_CR_SWRST;
	p_twi->TWI_RHR;
}

/**
 * \brief Get TWI PDC base address.
 *
 * \param p_twi Pointer to a TWI instance.
 *
 * \return TWI PDC registers base for PDC driver to access.
 */
Pdc *twi_get_pdc_base(Twi *p_twi)
{
	Pdc *p_pdc_base = NULL;
#if !SAMG
	if (p_twi == TWI0) {
		p_pdc_base = PDC_TWI0;
	} else
#endif
#ifdef PDC_TWI1
	 if (p_twi == TWI1) {
		p_pdc_base = PDC_TWI1;
	} else
#endif
#ifdef PDC_TWI2
	if (p_twi == TWI2) {
		p_pdc_base = PDC_TWI2;
	} else
#endif
	{
		Assert(false);
	}

	return p_pdc_base;
}

#if (SAM4E || SAM4C || SAMG || SAM4CP || SAM4CM)
/**
 * \brief Enables/Disables write protection mode.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param flag ture for enable, false for disable.
 */
void twi_set_write_protection(Twi *p_twi, bool flag)
{

	p_twi->TWI_WPMR = (flag ? TWI_WPMR_WPEN : 0) | TWI_WP_KEY_VALUE;
}

/**
 * \brief Read the write protection status.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param p_status Pointer to save the status.
 */
void twi_read_write_protection_status(Twi *p_twi, uint32_t *p_status)
{

	*p_status = p_twi->TWI_WPSR;
}
#endif

#if SAMG55
/**
 * \brief Set the prescaler, TLOW:SEXT, TLOW:MEXT and clock high max cycles for SMBUS mode.
 *
 * \param p_twi   Base address of the TWI instance.
 * \param ul_timing Parameter for prescaler, TLOW:SEXT, TLOW:MEXT and clock high max cycles.
 */
void twi_smbus_set_timing(Twi *p_twi, uint32_t ul_timing)
{
	p_twi->TWI_SMBTR = ul_timing;;
}

/**
 * \brief Set length/direction/PEC for alternative command mode.
 *
 * \param p_twi   Base address of the TWI instance.
 * \param ul_alt_cmd Alternative command parameters.
 */
void twi_set_alternative_command(Twi *p_twi, uint32_t ul_alt_cmd)
{
	p_twi->TWI_ACR = ul_alt_cmd;;
}

/**
 * \brief Set the filter for TWI.
 *
 * \param p_twi   Base address of the TWI instance.
 * \param ul_filter   Filter value.
 */
void twi_set_filter(Twi *p_twi, uint32_t ul_filter)
{
	p_twi->TWI_FILTR = ul_filter;;
}

/**
 * \brief A mask can be applied on the slave device address in slave mode in order to allow multiple
 * address answer. For each bit of the MASK field set to one the corresponding SADR bit will be masked.
 *
 * \param p_twi   Base address of the TWI instance.
 * \param ul_mask  Mask value.
 */
void twi_mask_slave_addr(Twi *p_twi, uint32_t ul_mask)
{
	p_twi->TWI_SMR |= TWI_SMR_MASK(ul_mask);
}

/**
 * \brief Set sleepwalking match mode.
 *
 * \param p_twi Pointer to a TWI instance.
 * \param ul_matching_addr1   Address 1 value.
 * \param ul_matching_addr2   Address 2 value.
 * \param ul_matching_addr3   Address 3 value.
 * \param ul_matching_data   Data value.
 * \param flag1 ture for set, false for no.
 * \param flag2 ture for set, false for no.
 * \param flag3 ture for set, false for no.
 * \param flag ture for set, false for no.
 */
void twi_set_sleepwalking(Twi *p_twi,
		uint32_t ul_matching_addr1, bool flag1,
		uint32_t ul_matching_addr2, bool flag2,
		uint32_t ul_matching_addr3, bool flag3,
		uint32_t ul_matching_data, bool flag)
{
	uint32_t temp = 0;

	if (flag1) {
		temp |= TWI_SWMR_SADR1(ul_matching_addr1);
	}

	if (flag2) {
		temp |= TWI_SWMR_SADR2(ul_matching_addr2);
	}

	if (flag3) {
		temp |= TWI_SWMR_SADR3(ul_matching_addr3);
	}

	if (flag) {
		temp |= TWI_SWMR_DATAM(ul_matching_data);
	}

	p_twi->TWI_SWMR = temp;
}
#endif
//@}

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
