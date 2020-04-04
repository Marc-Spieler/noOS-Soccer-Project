/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 30.03.2020                                                  */
/************************************************************************/

#include "ric.h"
#include "string.h"
#include "timing.h"

motor2Sensor_t m2s;
sensor2Motor_t s2m;
motor2Raspberrypi_t m2r;
raspberrypi2Motor_t r2m;

uint8_t sensBuf[sizeof(s2m)];
uint8_t rpiBuf[sizeof(r2m)];

Bool prepareNewData = false;
Bool newScDataArrived = false;
Bool newPiDataArrived = false;

// local functions
void spiMasterInit(void);
void spiSlaveInit(void);
void spi_master_transfer(void *p_buf, uint32_t ul_size);
void spi_slave_transfer(void *p_buf, uint32_t ul_size);
void configure_dmac(void);

void ricInit(void)
{
    configure_dmac();
    spiMasterInit();
    spiSlaveInit();
}

void spiMasterInit(void)
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

void spiSlaveInit(void)
{
    dmac_channel_disable(DMAC, 4);
    dmac_channel_disable(DMAC, 2);
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
    memcpy(&rpiBuf, &m2r, sizeof(m2r));
    spi_slave_transfer(&rpiBuf, sizeof(rpiBuf));
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

void spi_slave_transfer(void *p_buf, uint32_t ul_size)
{
    dma_transfer_descriptor_t dmac_trans;
    
    /* required to ensure proper byte alignment between master and slave */
    uint8_t dummyRead = SPI0->SPI_RDR;
    (void)dummyRead;

    dmac_channel_disable(DMAC, 4);
    dmac_trans.ul_source_addr = (uint32_t) p_buf;
    dmac_trans.ul_destination_addr = (uint32_t) & SPI0->SPI_TDR;
    dmac_trans.ul_ctrlA = ul_size | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
    dmac_trans.ul_ctrlB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_MEM2PER_DMA_FC | DMAC_CTRLB_SRC_INCR_INCREMENTING | DMAC_CTRLB_DST_INCR_FIXED;
    dmac_trans.ul_descriptor_addr = 0;
    dmac_channel_single_buf_transfer_init(DMAC, 4, &dmac_trans);
    dmac_channel_enable(DMAC, 4);

    dmac_channel_disable(DMAC, 2);
    dmac_trans.ul_source_addr = (uint32_t) & SPI0->SPI_RDR;
    dmac_trans.ul_destination_addr = (uint32_t) p_buf;
    dmac_trans.ul_ctrlA = ul_size | DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
    dmac_trans.ul_ctrlB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_PER2MEM_DMA_FC | DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_INCREMENTING;
    dmac_trans.ul_descriptor_addr = 0;
    dmac_channel_single_buf_transfer_init(DMAC, 2, (dma_transfer_descriptor_t *) & dmac_trans);
    dmac_channel_enable(DMAC, 2);
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

    /* Configure DMA RX channel. */
    ul_cfg = 0;
    ul_cfg |= DMAC_CFG_SRC_PER(2) | DMAC_CFG_SRC_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
    dmac_channel_set_configuration(DMAC, 2, ul_cfg);

    /* Configure DMA TX channel. */
    ul_cfg = 0;
    ul_cfg |= DMAC_CFG_DST_PER(1) | DMAC_CFG_DST_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
    dmac_channel_set_configuration(DMAC, 4, ul_cfg);

    /* Enable receive channel interrupt for DMAC. */
    NVIC_EnableIRQ(DMAC_IRQn);
    dmac_enable_interrupt(DMAC, (1 << 0));
    dmac_enable_interrupt(DMAC, (1 << 2));
}

void DMAC_Handler(void)
{
    static uint32_t dmaStatus;
    //static uint32_t prev_update = 0;

    dmaStatus = dmac_get_status(DMAC);
    
    if(dmaStatus & (1 << 0))
    {
        usart_spi_release_chip_select(USART1);
        memcpy(&s2m, &sensBuf, sizeof(s2m));
        newScDataArrived = true;
    }
    
    if(dmaStatus & (1 << 2))
    {
        memcpy(&r2m, &rpiBuf, sizeof(r2m));
        prepareNewData = true;
        newPiDataArrived = true;
        //s.camera_fps = (s.camera_fps * 0.9) + (1000 / (getTicks() - prev_update) * 0.1);
        //prev_update = getTicks();
    }
}

void ricMaintenance(void)
{
    static uint32_t m2sTicks = 0;
    
    if((getTicks() - m2sTicks) > 8)
    {
        m2sTicks= getTicks();
        
        memcpy(&sensBuf, &m2s, sizeof(m2s));
        spi_master_transfer(&sensBuf, sizeof(sensBuf));
    }
    
    if(prepareNewData)
    {
        prepareNewData = false;
        
        m2r.rsvd = 1234;
        
        memcpy(&rpiBuf, &m2r, sizeof(m2r));
        spi_slave_transfer(&rpiBuf, sizeof(rpiBuf));
    }
}
