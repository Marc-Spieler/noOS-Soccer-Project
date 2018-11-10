/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 19.08.18                                                    */
/************************************************************************/

#include "comm.h"
#include "string.h"

motor_to_sensor_t mts;
sensor_to_motor_t stm;

uint8_t sens_buf[sizeof(mts)];

void spi_init(void)
{
    configure_dmac();
    spi_master_initialize();
}

void spi_master_initialize(void)
{
    dmac_channel_disable(DMAC, 1);
    dmac_channel_disable(DMAC, 0);
    pmc_enable_periph_clk(ID_USART1);
    usart_spi_disable(USART1);

    usart_spi_opt_t spi_settings =
    {
        .baudrate = 1000000,
        .char_length = US_MR_CHRL_8_BIT,
        .spi_mode = SPI_MODE_0,
        .channel_mode = US_MR_CHMODE_NORMAL
    };
    usart_init_spi_master(USART1, &spi_settings, sysclk_get_peripheral_hz());
    usart_spi_enable(USART1);
}

void spi_master_transfer(void *p_buf, uint32_t ul_size)
{
    dma_transfer_descriptor_t dmac_trans;

    usart_spi_force_chip_select(USART1);

    dmac_channel_disable(DMAC, 1);
    dmac_trans.ul_source_addr = (uint32_t) p_buf;
    dmac_trans.ul_destination_addr = (uint32_t) & USART1->US_THR;
    dmac_trans.ul_ctrlA = ul_size | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
    dmac_trans.ul_ctrlB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_MEM2PER_DMA_FC | DMAC_CTRLB_SRC_INCR_INCREMENTING | DMAC_CTRLB_DST_INCR_FIXED;
    dmac_trans.ul_descriptor_addr = 0;
    dmac_channel_single_buf_transfer_init(DMAC, 1, &dmac_trans);
    dmac_channel_enable(DMAC, 1);

    dmac_channel_disable(DMAC, 0);
    dmac_trans.ul_source_addr = (uint32_t) & USART1->US_RHR;
    dmac_trans.ul_destination_addr = (uint32_t) p_buf;
    dmac_trans.ul_ctrlA = ul_size | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
    dmac_trans.ul_ctrlB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_PER2MEM_DMA_FC | DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_INCREMENTING;
    dmac_trans.ul_descriptor_addr = 0;
    dmac_channel_single_buf_transfer_init(DMAC, 0, (dma_transfer_descriptor_t *) & dmac_trans);
    dmac_channel_enable(DMAC, 0);
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
    ul_cfg |= DMAC_CFG_SRC_PER(14) | DMAC_CFG_SRC_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
    dmac_channel_set_configuration(DMAC, 0, ul_cfg);

    /* Configure DMA TX channel. */
    ul_cfg = 0;
    ul_cfg |= DMAC_CFG_DST_PER(13) | DMAC_CFG_DST_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
    dmac_channel_set_configuration(DMAC, 1, ul_cfg);
    #if 0
    /* Configure DMA RX channel. */
    ul_cfg = 0;
    ul_cfg |= DMAC_CFG_SRC_PER(2) | DMAC_CFG_SRC_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
    dmac_channel_set_configuration(DMAC, 2, ul_cfg);

    /* Configure DMA TX channel. */
    ul_cfg = 0;
    ul_cfg |= DMAC_CFG_DST_PER(1) | DMAC_CFG_DST_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
    dmac_channel_set_configuration(DMAC, 4, ul_cfg);
    #endif
    /* Enable receive channel interrupt for DMAC. */
    NVIC_EnableIRQ(DMAC_IRQn);
    dmac_enable_interrupt(DMAC, (1 << 0));
}

void DMAC_Handler(void)
{
    static uint32_t ul_status;

    ul_status = dmac_get_status(DMAC);
    
    if (ul_status & (1 << 0))
    {
        usart_spi_release_chip_select(USART1);
        memcpy(&stm, &sens_buf, sizeof(stm));
    }
}
