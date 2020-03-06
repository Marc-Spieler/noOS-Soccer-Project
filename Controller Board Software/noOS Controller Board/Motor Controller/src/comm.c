/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 19.08.2018                                                  */
/************************************************************************/

#include "comm.h"
#include "string.h"
#include "timing.h"

sensors_t s;
motor_to_sensor_t mts;
sensor_to_motor_t stm;
motor_to_raspberrypi_t mtr;
raspberrypi_to_motor_t rtm;

uint8_t sens_buf[sizeof(stm)];
uint8_t rpi_buf[sizeof(rtm)];

static Bool prepare_new_values = false;
static Bool new_sc_data_arrived = false;
static Bool new_pi_data_arrived = false;

void spi_init(void)
{
    configure_dmac();
    spi_master_initialize();
    spi_slave_initialize();
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

void spi_slave_initialize(void)
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
    memcpy(&rpi_buf, &mtr, sizeof(mtr));
    spi_slave_transfer(&rpi_buf, sizeof(rpi_buf));
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
    static uint32_t ul_status;
    static uint32_t prev_update = 0;

    ul_status = dmac_get_status(DMAC);
    
    if (ul_status & (1 << 0))
    {
        usart_spi_release_chip_select(USART1);
        memcpy(&stm, &sens_buf, sizeof(stm));
        new_sc_data_arrived = true;
    }
    
    if (ul_status & (1 << 2))
    {
        memcpy(&rtm, &rpi_buf, sizeof(rtm));
        prepare_new_values = true;
        new_pi_data_arrived = true;
        s.camera_fps = (s.camera_fps * 0.9) + (1000 / (getTicks() - prev_update) * 0.1);
        prev_update = getTicks();
    }
}

void prepare_values_to_send(void)
{
    if (prepare_new_values)
    {
        prepare_new_values = false;
        
        mtr.rsvd = 1234;
        
        memcpy(&rpi_buf, &mtr, sizeof(mtr));
        spi_slave_transfer(&rpi_buf, sizeof(rpi_buf));
    }
}

void process_new_sensor_values(void)
{
    static uint32_t last_pi_update = 0;
    
    static uint32_t ticks_line_seen = 0;
    static uint32_t ticks_ball_seen = 0;
    static uint32_t ticks_ball_have = 0;
    static uint32_t ticks_ball_have_2 = 0;
    static uint32_t ticks_goal_seen = 0;
    static uint16_t prev_esc_dir = 0;
    static uint8_t prev_diff = 0;
    static int8_t prev_s_ball_dir = 0;
    static int8_t prev_s_goal_dir = 0;
    static uint8_t prev_s_goal_diff = 0;
    /*static float ball_see_avg = 0.0f;
    static float ball_have_avg = 0.0f;
    static float ball_have_2_avg = 0.0f;
    static float goal_see_avg = 0.0f;
    float ball_see_tmp = 0.0f;
    float ball_have_tmp = 0.0f;
    float ball_have_2_tmp = 0.0f;
    float goal_see_tmp = 0.0f;*/
    
    if((getTicks() - last_pi_update) > 100)
    {
        s.rpi_inactive = true;
        ioport_set_pin_level(LED_M3, 1);
    }
    else
    {
        s.rpi_inactive = false;
        ioport_set_pin_level(LED_M3, 0);
    }
    
    if(new_sc_data_arrived)
    {
        new_sc_data_arrived = false;
        memcpy(&s.line.all, &stm.line.all, sizeof(s.line.all));
        //memcpy(&s.line.single, &stm.line.single, sizeof(s.line.single));
        
        if(stm.line.see)
        {
            ticks_line_seen = getTicks();
            s.line.see = 1;
            s.line.esc = stm.line.esc;
            while(s.line.esc > 180) s.line.esc -= 360;
            s.line.diff = stm.line.diff;
            prev_esc_dir = s.line.esc;
            prev_diff = s.line.diff;
        }
        else
        {
            if((getTicks() - ticks_line_seen) > 100)
            {
                s.line.see = 0;
                s.line.esc = 0;
                s.line.diff = 0;
            }
            else
            {
                s.line.see = 1;
                s.line.esc = prev_esc_dir;
                s.line.diff = prev_diff;
            }
        }
        
        memcpy(&s.distance, &stm.distance, sizeof(s.distance));
        memcpy(&s.battery, &stm.battery, sizeof(s.battery));
    }

    if (new_pi_data_arrived)
    {
        new_pi_data_arrived = false;
        last_pi_update = getTicks();

        /*ball_see_tmp = (rtm.ball.see) ? 0.1f : 0.0f;
        ball_have_tmp = (rtm.ball.have) ? 0.1f : 0.0f;
        ball_have_2_tmp = (rtm.ball.have_2) ? 0.1f : 0.0f;
        goal_see_tmp = (rtm.goal.see) ? 0.1f : 0.0f;

        ball_see_avg = ball_see_avg * 0.9 + ball_see_tmp;
        ball_have_avg = ball_have_avg * 0.9 + ball_have_tmp;
        ball_have_2_avg = ball_have_2_avg * 0.9 + ball_have_2_tmp;
        goal_see_avg = goal_see_avg * 0.9 + goal_see_tmp;

        s.ball.see = (ball_see_avg > 0.1) ? true : false;
        s.ball.have = (ball_have_avg > 0.1) ? true : false;
        s.ball.have_2 = (ball_have_2_avg > 0.1) ? true : false;
        s.goal.see = (goal_see_avg > 0.1) ? true : false;*/
        
        if(rtm.ball.see)
        {
            ticks_ball_seen = getTicks();
            s.ball.see = true;
            s.ball.dir = (rtm.ball.dir - 63);
			while(s.ball.dir > 180)
			{
				s.ball.dir -= 360;
			}
            prev_s_ball_dir = s.ball.dir;
        }
        else
        {
            if((getTicks() - ticks_ball_seen) > 100)
            {
                s.ball.see = false;
                s.ball.dir = 0;
            }
            else
            {
                s.ball.see = true;
                s.ball.dir = prev_s_ball_dir;
            }
        }
        
        if(rtm.ball.have)
        {
            ticks_ball_have = getTicks();
            s.ball.have = true;
        }
        else
        {
            if((getTicks() - ticks_ball_have) > 100)
            {
                s.ball.have = false;
            }
            else
            {
                s.ball.have = true;
            }
        }
        
        if(rtm.ball.have_2)
        {
            ticks_ball_have_2 = getTicks();
            s.ball.have_2 = true;
        }
        else
        {
            if((getTicks() - ticks_ball_have_2) > 100)
            {
                s.ball.have_2 = false;
            }
            else
            {
                s.ball.have_2 = true;
            }
        }
        
        if(rtm.goal.see)
        {
            ticks_goal_seen = getTicks();
            s.goal.see = true;
            s.goal.dir = (rtm.goal.dir - 63);
            s.goal.diff = rtm.goal.diff * 2;
            prev_s_goal_dir = s.goal.dir;
            prev_s_goal_diff = s.goal.diff;
        }
        else
        {
            if((getTicks() - ticks_goal_seen) > 100)
            {
                s.goal.see = false;
                s.goal.dir = 0;
                s.goal.diff = 0;
            }
            else
            {
                s.goal.see = true;
                s.goal.dir = prev_s_goal_dir;
                s.goal.diff = prev_s_goal_diff;
            }
        }

        /*if(rtm.ball.see)
        {
            s.ball.dir = (rtm.ball.dir - 32) * 2;
            prev_s_ball_dir = s.ball.dir;
        }
        else
        {
            s.ball.dir = prev_s_ball_dir;
        }

        if(rtm.goal.see)
        {
            s.goal.dir = (rtm.goal.dir - 32) * 2;
            s.goal.diff = rtm.goal.diff * 2;
            prev_s_goal_dir = s.goal.dir;
            prev_s_goal_diff = s.goal.diff;
        }
        else
        {
            s.goal.dir = prev_s_goal_dir;
            s.goal.diff = prev_s_goal_diff;
        }*/
    }
}
