/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 21.08.2018                                                  */
/************************************************************************/

#include "comm.h"
#include "string.h"

motor_to_sensor_t mts;
sensor_to_motor_t stm;

static Bool received_data = 0;
uint8_t sens_buf[sizeof(stm)];

void spi_init(void)
{
    configure_dmac();
    spi_slave_initialize();
}

void spi_slave_transfer(void *p_buf, uint32_t ul_size)
{
    dma_transfer_descriptor_t dmac_trans;

    /* required to ensure proper byte alignment between master and slave */
    uint8_t dummyRead = SPI0->SPI_RDR;
    (void)dummyRead;

    dmac_channel_disable(DMAC, 1);
    dmac_trans.ul_source_addr = (uint32_t) p_buf;
    dmac_trans.ul_destination_addr = (uint32_t) & SPI0->SPI_TDR;
    dmac_trans.ul_ctrlA = ul_size | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
    dmac_trans.ul_ctrlB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_MEM2PER_DMA_FC | DMAC_CTRLB_SRC_INCR_INCREMENTING | DMAC_CTRLB_DST_INCR_FIXED;
    dmac_trans.ul_descriptor_addr = 0;
    dmac_channel_single_buf_transfer_init(DMAC, 1, &dmac_trans);
    dmac_channel_enable(DMAC, 1);

    dmac_channel_disable(DMAC, 0);
    dmac_trans.ul_source_addr = (uint32_t) & SPI0->SPI_RDR;
    dmac_trans.ul_destination_addr = (uint32_t) p_buf;
    dmac_trans.ul_ctrlA = ul_size | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
    dmac_trans.ul_ctrlB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_PER2MEM_DMA_FC | DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_INCREMENTING;
    dmac_trans.ul_descriptor_addr = 0;
    dmac_channel_single_buf_transfer_init(DMAC, 0, (dma_transfer_descriptor_t *) & dmac_trans);
    dmac_channel_enable(DMAC, 0);
}

void spi_slave_initialize(void)
{
    dmac_channel_disable(DMAC, 1);
    dmac_channel_disable(DMAC, 0);
    pmc_enable_periph_clk(ID_SPI0);
    spi_disable(SPI0);
    spi_reset(SPI0);
    spi_set_slave_mode(SPI0);
    spi_disable_mode_fault_detect(SPI0);
    spi_set_peripheral_chip_select_value(SPI0, spi_get_pcs(0));
    spi_set_clock_polarity(SPI0, 0, 0);
    spi_set_clock_phase(SPI0, 0, 1);
    spi_set_bits_per_transfer(SPI0, 0, SPI_CSR_BITS_8_BIT);
    spi_enable(SPI0);

    /* Start waiting command. */
    memcpy(&sens_buf, &stm, sizeof(stm));
    spi_slave_transfer(&sens_buf, sizeof(sens_buf));
}

void configure_dmac(void)
{
    uint32_t ul_cfg;

    /* Initialize and enable DMA controller. */
    pmc_enable_periph_clk(ID_DMAC);
    dmac_init(DMAC);
    dmac_set_priority_mode(DMAC, DMAC_PRIORITY_ROUND_ROBIN);
    dmac_enable(DMAC);

    /* Configure DMA RX channel. */
    ul_cfg = 0;
    ul_cfg |= DMAC_CFG_SRC_PER(2) |
    DMAC_CFG_SRC_H2SEL |
    DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
    dmac_channel_set_configuration(DMAC, 0, ul_cfg);

    /* Configure DMA TX channel. */
    ul_cfg = 0;
    ul_cfg |= DMAC_CFG_DST_PER(1) |
    DMAC_CFG_DST_H2SEL |
    DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
    dmac_channel_set_configuration(DMAC, 1, ul_cfg);

    /* Enable receive channel interrupt for DMAC. */
    NVIC_EnableIRQ(DMAC_IRQn);
    dmac_enable_interrupt(DMAC, (1 << 0));
}

void DMAC_Handler(void)
{
    static uint32_t ul_status;

    ul_status = dmac_get_status(DMAC);
    
    if (ul_status & 1)
    {
        memcpy(&mts, &sens_buf, sizeof(mts));
        received_data = true;
    }
}

void PrepareValuesToSend(void)
{
    if (received_data)
    {
        received_data = false;
        memcpy(&sens_buf, &stm, sizeof(stm));
        spi_slave_transfer(&sens_buf, sizeof(sens_buf));
    }
}