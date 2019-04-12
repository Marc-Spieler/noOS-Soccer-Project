/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 07.03.19                                                    */
/************************************************************************/

#include "line.h"
#include "comm.h"

#define LED_CNT 12
#define SEG_CNT 4

Bool line_white[LED_CNT];
const int16_t led_deg[LED_CNT] =
{
    -15, -45, -75, -105, -135, -165, -195, -225, -255, -285, -315, -345
};

int16_t seg_sta[SEG_CNT];
int16_t seg_end[SEG_CNT];
int16_t seg_mid[SEG_CNT];
int16_t line_esc;
int8_t seg_cntr;
uint8_t led_cntr;
int8_t divider;
uint32_t act_line_value;
Bool prev_led_white;

void dacc_init(void)
{
    sysclk_enable_peripheral_clock(ID_DACC);
    dacc_reset(DACC);
    dacc_set_timing(DACC, 0x08, 0, 0x10);
    dacc_enable_channel(DACC, DACC_CHANNEL_LINE_VREF);
    //dacc_enable_channel(DACC, DACC_CHANNEL_VREF_BLACK);
    dacc_set_analog_control(DACC, DACC_ANALOG_CONTROL);
    dacc_set_channel_selection(DACC, DACC_CHANNEL_LINE_VREF);
    dacc_write_conversion_data(DACC, 3000);
    /*dacc_set_channel_selection(DACC, DACC_CHANNEL_VREF_BLACK);
    dacc_write_conversion_data(DACC, 3000);*/
}

/*void set_vref_black(uint16_t value)
{
    dacc_set_channel_selection(DACC, DACC_CHANNEL_VREF_BLACK);
    dacc_write_conversion_data(DACC, value);
}*/

void set_vref_white(uint16_t value)
{
    dacc_set_channel_selection(DACC, DACC_CHANNEL_LINE_VREF);
    dacc_write_conversion_data(DACC, value);
}

void update_line_values(void)
{
    act_line_value = ioport_get_port_level(IOPORT_PIOC, 0x495AA888);
    
    stm.line.segment_1 = (act_line_value & 0x00020000) != 0x00020000 ? 1 : 0;
    stm.line.segment_2 = (act_line_value & 0x00080000) != 0x00080000 ? 1 : 0;
    stm.line.segment_3 = (act_line_value & 0x40000000) != 0x40000000 ? 1 : 0;
    stm.line.segment_4 = (act_line_value & 0x00100000) != 0x00100000 ? 1 : 0;
    stm.line.segment_5 = (act_line_value & 0x00400000) != 0x00400000 ? 1 : 0;
    stm.line.segment_6 = (act_line_value & 0x01000000) != 0x01000000 ? 1 : 0;
    stm.line.segment_7 = (act_line_value & 0x08000000) != 0x08000000 ? 1 : 0;
    stm.line.segment_8 = (act_line_value & 0x00000008) != 0x00000008 ? 1 : 0;
    stm.line.segment_9 = (act_line_value & 0x00000080) != 0x00000080 ? 1 : 0;
    stm.line.segment_10 = (act_line_value & 0x00000800) != 0x00000800 ? 1 : 0;
    stm.line.segment_11 = (act_line_value & 0x00002000) != 0x00002000 ? 1 : 0;
    stm.line.segment_12 = (act_line_value & 0x00008000) != 0x00008000 ? 1 : 0;
    
}

void calculate_line_esc_direction(void)
{
    if (act_line_value != 0x495AA888)
    {
        line_white[0] = stm.line.segment_1;
        line_white[1] = stm.line.segment_2;
        line_white[2] = stm.line.segment_3;
        line_white[3] = stm.line.segment_4;
        line_white[4] = stm.line.segment_5;
        line_white[5] = stm.line.segment_6;
        line_white[6] = stm.line.segment_7;
        line_white[7] = stm.line.segment_8;
        line_white[8] = stm.line.segment_9;
        line_white[9] = stm.line.segment_10;
        line_white[10] = stm.line.segment_11;
        line_white[11] = stm.line.segment_12;
        
        for (seg_cntr = 0; seg_cntr < SEG_CNT; seg_cntr++)
        {
            seg_sta[seg_cntr] = 0;
            seg_end[seg_cntr] = 0;
        }

        for (led_cntr = 0; led_cntr < LED_CNT; led_cntr++)
        {
            if (!line_white[led_cntr])
            {
                break;
            }
        }
        
        seg_cntr = 0;
        prev_led_white = 0;
        for (int16_t i = 0; i < LED_CNT; i++)
        {
            int16_t j = (led_cntr + i) % LED_CNT;
            if (line_white[j] && !prev_led_white)
            {
                seg_sta[seg_cntr] = led_deg[j];
                if (seg_end[seg_cntr] != 0)
                {
                    seg_cntr++;
                }
            }

            if (!line_white[j] && prev_led_white)
            {
                seg_end[seg_cntr] = led_deg[j-1];
                seg_cntr++;
            }
            
            if (line_white[j] && (i == LED_CNT-1))
            {
                seg_end[seg_cntr] = led_deg[j];
                seg_cntr++;
            }
            
            prev_led_white = line_white[j];
        }
        
        divider = seg_cntr;
        
        for (seg_cntr = 0; seg_cntr < SEG_CNT; seg_cntr++)
        {
            if (seg_sta[seg_cntr] < seg_end[seg_cntr])
            {
                seg_sta[seg_cntr] += 360;
            }
            seg_mid[seg_cntr] = (seg_sta[seg_cntr] + seg_end[seg_cntr]) / 2;
        }
        
        if (divider == 2)
        {
            if (seg_mid[0] - seg_mid[1] > 180)
            {
                line_esc = (seg_mid[0] + seg_mid[1]) / divider;
            }
            else
            {
                line_esc = (seg_mid[0] + seg_mid[1]) / divider;
                line_esc += 180;
            }
        }
        else if (divider > 0)
        {
            line_esc = 0;
            for (seg_cntr = 0; seg_cntr < SEG_CNT; seg_cntr++)
            {
                line_esc += seg_mid[seg_cntr];
            }
            line_esc /= divider;
            line_esc += 180;
        }
        else if (divider == 0)
        {
            line_esc = seg_mid[0] + 180;
        }
        
        stm.line.see = 1;
        set_led(LED_ONBOARD, 1);
        set_led(LED_S3, 1);
    }
    else
    {
        line_esc = 0;
        stm.line.see = 0;
        set_led(LED_ONBOARD, 0);
        set_led(LED_S3, 0);
    }
    
    if (line_esc >= 180)
    {
        line_esc -= 360;
    }
    
    if (line_esc <= -180)
    {
        line_esc += 360;
    }
    
    stm.line_esc = line_esc;
    
}

/*if ((getTicks() - ul_ticks_vref) > VREF_UPDATE_RATE)
{
    ul_ticks_vref = getTicks();
        
    if (sensor_parameters.vref_black == 27)
    {
        ioport_set_pin_level(S1_LED, 1);
    }
    else
    {
        ioport_set_pin_level(S1_LED, 0);
    }
        
    set_vref_black(sensor_parameters.vref_black * 65);
    set_vref_white(sensor_parameters.vref_white * 65);
}*/
