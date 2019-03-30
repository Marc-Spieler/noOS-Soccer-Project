/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 30.03.2019                                                  */
/************************************************************************/

#include "sd.h"

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
    uint32_t bytes_written;
    uint32_t start_time = getTicks();
    set_led(LED_ONBOARD, 0);

    for(int i = 0; i < 10000; i++)
    {
        f_write(&file_object, "Test SD/MMC stack\n", 18, &bytes_written);
    }
    
    uint32_t diff = getTicks() - start_time;
    close_file_match;
}

void write_time_test(void)
{
    res = f_open(&file_object, (char const *)test_file_name, FA_CREATE_ALWAYS | FA_WRITE);
    
    if (res != FR_OK)
    {
        while(1);
    }
    
    uint32_t diff = 0;
    uint32_t bytes_written;

    set_led(LED_M3, 0);

    for(int i = 0; i < 1000; i++)
    {
        uint32_t start_time = getTicks();
        set_led(LED_ONBOARD, 1);

        f_write(&file_object, diff, 18, &bytes_written);
        
        set_led(LED_ONBOARD, 0);
        diff = getTicks() - start_time;
    }

    set_led(LED_M3, 1);
    f_close(&file_object);
}