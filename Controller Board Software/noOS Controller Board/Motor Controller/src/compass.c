/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 01.04.2020                                                  */
/************************************************************************/

#include "compass.h"
#include "string.h"
#include "data.h"
#include "timing.h"

#define BNO055_ID                   (0xA0)
#define POWER_MODE_NORMAL           (0x00)
#define OPERATION_MODE_CONFIG       (0x00)
#define OPERATION_MODE_NDOF         (0x0C)

#define BNO055_CHIP_ID_ADDR         (0x00)
#define BNO055_PAGE_ID_ADDR         (0x07)
#define BNO055_OPR_MODE_ADDR        (0x3D)
#define BNO055_PWR_MODE_ADDR        (0x3E)
#define BNO055_SYS_TRIGGER_ADDR     (0x3F)
#define BNO055_EULER_H_LSB_ADDR     (0x1A)
#define BNO055_CALIB_STAT_ADDR      (0x35)
#define BNO055_UNIT_SEL_ADDR        (0x3B)

uint16_t rawCompass;
uint16_t goalReferencePoint;

static uint32_t lastCompassRead = 0;
static uint8_t compassIsBusy = false;
static uint32_t compassStartTicks = 0;
static uint32_t compassTimeoutErrorCntr = 0;
static twi_master_options_t twiConfig;

// local functions
static void compass_write_byte(uint8_t addr, uint8_t data);
static uint8_t compass_read_block(uint8_t addr, uint8_t *data, uint8_t count);
static void compassCallback(void);

void compassInit(void)
{
    twi_set_compass_tx_callback(compassCallback);
    twi_set_compass_rx_callback(compassCallback);

    memset((void *)&twiConfig, 0, sizeof(twiConfig));
    twiConfig.speed = 100000;
    twi_master_setup(TWI0, &twiConfig);
    
    // Switch to config mode (just in case since this is the default)
    compass_write_byte(BNO055_PAGE_ID_ADDR, 0x00);
    // Reset
    compass_write_byte(BNO055_SYS_TRIGGER_ADDR, 0x20);
    // Delay increased to 30ms due to power issues https://tinyurl.com/y375z699
    mdelay(1000);
    uint8_t chip_id;
    do
    {
        compass_read_block(BNO055_CHIP_ID_ADDR, &chip_id, 1);
        mdelay(10);
    }
    while(chip_id != BNO055_ID);
    // Set to normal power mode
    compass_write_byte(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
    // Select external crystal
    compass_write_byte(BNO055_SYS_TRIGGER_ADDR, 0x80);
    mdelay(100);
    // Set the requested operating mode (see section 3.3)
    compass_write_byte(BNO055_OPR_MODE_ADDR, OPERATION_MODE_NDOF);
    mdelay(100);
    // Euler from -180? to +180? increasing clockwise
    compass_write_byte(BNO055_UNIT_SEL_ADDR, 0x00);
    mdelay(100);
}

void compassMaintenance(void)
{
    if((getTicks() - lastCompassRead) >= 20)
    {
        lastCompassRead = getTicks();
        
        // Get a Euler angle sample for orientation
        uint8_t euler_head[2];
        if(compass_read_block(BNO055_EULER_H_LSB_ADDR, &euler_head[0], 2) == 0)
        {
            rawCompass = (euler_head[1] << 8) | euler_head[0];     // 0&1=x; 2&3=y; 4&5=z
            // the following lines divide the value by 1.6 to match
            // the output of the previous sensor
            rawCompass *= 10;
            rawCompass >>= 4;    // = divide by 16
        }
        
        if((int16_t)goalReferencePoint == 0) setGoalReference(false);
        
        float goalDeviation = ((float)(rawCompass - goalReferencePoint) / 10.0f);

        while(goalDeviation > 180.0f) goalDeviation -= 360.0f;
        while(goalDeviation <= -180.0f) goalDeviation += 360.0f;
        
        data.compass = goalDeviation;
    }
}

uint8_t get_compass_cal_status(uint8_t *cal_status)
{
    return compass_read_block(BNO055_CALIB_STAT_ADDR, cal_status, 1);
}

void setGoalReference(Bool inverted)
{
    if(inverted) goalReferencePoint = (rawCompass - 1800);
    else goalReferencePoint = rawCompass;
}

void compassCalibrationStep(void)
{
    twi_packet_t *tx_packet = twi_get_tx_packet();
    
    tx_packet->chip = 0x60;
    tx_packet->addr[0] = 0x0f;
    tx_packet->addr_length = 1;
    
    tx_packet->buffer[0] = 0xff;
    tx_packet->length = 1;
    
    twi_pdc_master_write(TWI0, tx_packet);
}

/************************************************************
* Local Functions                                           *
************************************************************/
static void compass_write_byte(uint8_t addr, uint8_t byte)
{
    twi_packet_t *tx_packet = twi_get_tx_packet();
    
    tx_packet->chip = 0x28;
    tx_packet->addr[0] = addr;
    tx_packet->addr_length = 1;
    tx_packet->buffer[0] = byte;
    tx_packet->length = 1;

    compassIsBusy = true;
    twi_pdc_master_write(TWI0, tx_packet);
    while(compassIsBusy);
}

static uint8_t compass_read_block(uint8_t addr, uint8_t *block, uint8_t count)
{
    twi_packet_t *rx_packet = twi_get_rx_packet();
    
    rx_packet->chip = 0x28;
    rx_packet->addr[0] = addr;
    rx_packet->addr_length = 1;
    rx_packet->length = count;

    compassStartTicks = getTicks();
    compassIsBusy = true;
    
    if(twi_pdc_master_read(TWI0, rx_packet) == TWI_SUCCESS)
    {
        while(compassIsBusy || twi_is_busy())
        {
            if((getTicks() - compassStartTicks) > 3)
            {
                compassIsBusy = false;
                compassTimeoutErrorCntr++;
                pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS | PERIPH_PTCR_RXTDIS);
                twi_master_setup(TWI0, &twiConfig);
                return 1;
            }
        }
    }

    memcpy(block, rx_packet->buffer, count);
    return 0;
}

static void compassCallback(void)
{
    compassIsBusy = false;
}
