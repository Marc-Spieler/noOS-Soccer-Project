/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 30.03.2019                                                  */
/************************************************************************/

#include "sd.h"
#include "asf.h"
#include "string.h"
#include "timing.h"
#include "lcd.h"
#include "menu.h"
#include "comm.h"
#include "compass.h"

char test_file_name[] = "0:sd_mmc_test.txt";
Ctrl_status status;
FRESULT res;
FATFS fs;
FIL file_object;

void sd_init(void)
{
    do
    {
        status = sd_mmc_test_unit_ready(0);
        
        if (CTRL_FAIL == status)
        {
            while (CTRL_NO_PRESENT != sd_mmc_check(0));
        }
    } while (CTRL_GOOD != status);

    memset(&fs, 0, sizeof(FATFS));
    res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);
        
    if (FR_INVALID_DRIVE == res)
    {
        while(1);
    }

    test_file_name[0] = LUN_ID_SD_MMC_0_MEM + '0';
}

void open_file_match(void)
{
    res = f_open(&file_object, (char const *)test_file_name, FA_CREATE_ALWAYS | FA_WRITE);
    
    if (res != FR_OK)
    {
        while(1);
    }
}

void close_file_match(void)
{
    f_close(&file_object);
}

void write_data(void)
{
    UINT bytes_written;
    //uint32_t start_time = getTicks();
    ioport_set_pin_level(LED_ONBOARD, 0);

    for(int i = 0; i < 10000; i++)
    {
        f_write(&file_object, "Test SD/MMC stack\n", 18, &bytes_written);
    }
    
    //uint32_t diff = getTicks() - start_time;
    close_file_match();
}

void write_time_test(void)
{
    res = f_open(&file_object, (char const *)test_file_name, FA_CREATE_ALWAYS | FA_WRITE);
    
    if (res != FR_OK)
    {
        while(1);
    }

    UINT bw;
    uint32_t sta;
    uint32_t diff = 0;
    char sprintf_buf[8];
    ioport_set_pin_level(LED_ONBOARD, 0);
    for(int i = 0; i < 10000; i++)
    {
        sta = getTicks();
        sprintf(sprintf_buf, "%4d\r\n", (int)diff);
        f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
        //f_sync(&file_object);
        diff = getTicks() - sta;
    }
    sta = getTicks();
    f_sync(&file_object);
    diff = getTicks() - sta;
    f_write(&file_object, "\r\n", 2, &bw);
    sprintf(sprintf_buf, "%4d\r\n", (int)diff);
    f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
    f_close(&file_object);
    ioport_set_pin_level(LED_ONBOARD, 1);
    while(1);
}

void write_time_test_2(void)
{
    uint8_t m1_act = 255;
    uint8_t m1_ref = 254;
    uint8_t m2_act = 253;
    uint8_t m2_ref = 252;
    uint8_t m3_act = 251;
    uint8_t m3_ref = 250;

    res = f_open(&file_object, (char const *)test_file_name, FA_CREATE_ALWAYS | FA_WRITE);
    
    if (res != FR_OK)
    {
        while(1);
    }

    UINT bw;
    uint32_t sta;
    uint32_t diff = 0;
    char sprintf_buf[11];
    ioport_set_pin_level(LED_ONBOARD, 0);
    for(int i = 0; i < 10000; i++)
    {
        sta = getTicks();
        sprintf(sprintf_buf, "%6d;", i);
        f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
        sprintf(sprintf_buf, "%4d;", (int)diff);
        f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);

        sprintf(sprintf_buf, "%8d;", (int)m1_act);
        f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
        sprintf(sprintf_buf, "%8d;", (int)m1_ref);
        f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);

        sprintf(sprintf_buf, "%8d;", (int)m2_act);
        f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
        sprintf(sprintf_buf, "%8d;", (int)m2_ref);
        f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);

        sprintf(sprintf_buf, "%8d;", (int)m3_act);
        f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
        sprintf(sprintf_buf, "%8d;\r\n", (int)m3_ref);
        f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
        //f_sync(&file_object);
        diff = getTicks() - sta;
    }
    sta = getTicks();
    f_sync(&file_object);
    diff = getTicks() - sta;
    f_write(&file_object, "\r\n", 2, &bw);
    sprintf(sprintf_buf, "%4d\r\n", (int)diff);
    f_write(&file_object, sprintf_buf, strlen(sprintf_buf), &bw);
    f_close(&file_object);
    ioport_set_pin_level(LED_ONBOARD, 1);
    while(1);
}
