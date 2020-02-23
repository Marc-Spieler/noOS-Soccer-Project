/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 29.01.18                                                    */
/************************************************************************/

#include "lcd.h"
#include "timing.h"
#include "string.h"

/************************************************************
* Local Definitions                                         *
************************************************************/
#define LCD_ENABLE_HIGH       0x04
#define LCD_ENABLE_LOW        0x00
#define LCD_WRITE_DDR         0x01
#define LCD_FUNTION_WRITE     0x00

#define LCD_INIT_WAIT_TIME    6         // ms
#define LCD_CLEAR_WAIT_TIME   3         // ms
#define LCD_TIMEOUT_DELAY     10        // ms

/************************************************************
* Local Variables                                           *
************************************************************/
static Bool lcd_in_use = true;
static backlight_t backlight;
static Bool lcdIsBusy = false;
static uint32_t lcdStartTicks = 0;
static uint32_t lcdTimeoutErrorCntr = 0;
static twi_master_options_t twiConfig;

/************************************************************
* Local Function Prototypes                                 *
************************************************************/
static void send_nibble(uint8_t cmd, uint8_t byte);
static void send_byte(uint8_t cmd, uint8_t byte);
static void lcd_callback(void);

/************************************************************
* Functions                                                 *
************************************************************/
void lcd_init(void)
{
    twi_set_lcd_tx_callback(lcd_callback);

    memset((void *)&twiConfig, 0, sizeof(twiConfig));
    twiConfig.speed = 100000;

    backlight = LCD_LIGHT_OFF;

    send_nibble(LCD_FUNTION_WRITE, 0x03);
    mdelay(LCD_INIT_WAIT_TIME);
    send_nibble(LCD_FUNTION_WRITE, 0x03);
    mdelay(LCD_INIT_WAIT_TIME);
    send_nibble(LCD_FUNTION_WRITE, 0x03);
    mdelay(LCD_INIT_WAIT_TIME);
    send_nibble(LCD_FUNTION_WRITE, 0x02);     // Interface: 4-Bit
    send_byte(LCD_FUNTION_WRITE, 0x28);       // Interface: 4-Bit, 2-lines, 5x8-dots
    send_byte(LCD_FUNTION_WRITE, 0x08);       // Display off, cursor off, cursor is underline
    send_byte(LCD_FUNTION_WRITE, 0x01);       // Clear display
    mdelay(LCD_CLEAR_WAIT_TIME);
    send_byte(LCD_FUNTION_WRITE, 0x06);       // Cursor moves right, no display shift
    send_byte(LCD_FUNTION_WRITE, 0x0C);       // Display on, cursor off, cursor is underline

    backlight = LCD_LIGHT_ON;
}

void lcd_set_backlight(backlight_t state)
{
    backlight = state;
//    send_byte(LCD_FUNTION_WRITE, 0x01);   // clear display
//    mdelay(LCD_CLEAR_WAIT_TIME);
}

void lcd_clear(void)
{
    send_byte(LCD_FUNTION_WRITE, 0x01);   // clear display
    mdelay(LCD_CLEAR_WAIT_TIME);
}

void lcd_print_i(uint8_t line, uint8_t col, int32_t data)
{
    char str[11];

    sprintf(str, "%10ld", data);
    lcd_print_s(line, col, str);
}

void lcd_print_s(uint8_t line, uint8_t col, const char* str)
{
    twi_packet_t *tx_packet = twi_get_tx_packet();
    uint8_t addr;
    uint8_t cmd;
    uint8_t byte;
    uint8_t count;
    
    if(!lcd_in_use)
    {
        return;
    }
    
    while(lcdIsBusy | twi_is_busy())
    {
        if((getTicks() - lcdStartTicks) > LCD_TIMEOUT_DELAY)
        {
            lcdIsBusy = false;
            lcdTimeoutErrorCntr++;
            pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS | PERIPH_PTCR_RXTDIS);
            twi_master_setup(TWI0, &twiConfig);
            break;
        }
    }

    tx_packet->chip = 0x27;
    tx_packet->addr[0] = 0x00;
    tx_packet->addr_length = 0;
    
    // Calculate display address
    switch(line)
    {
        case 2:
        addr = 0x80 | 0x40;   // set address to start of line 2
        break;
        case 3:
        addr = 0x80 | 0x14;   // set address to start of line 3
        break;
        case 4:
        addr = 0x80 | 0x54;   // set address to start of line 4
        break;
        case 1:
        default:
        addr = 0x80;          // set address to start of line 1
        break;
    }
    addr += col;  // set column in the selected line

    // Set cursor
    cmd = LCD_FUNTION_WRITE | backlight;
    tx_packet->buffer[0] = cmd | LCD_ENABLE_HIGH | (addr & 0xF0);
    tx_packet->buffer[1] = cmd | LCD_ENABLE_LOW  | (addr & 0xF0);
    tx_packet->buffer[2] = cmd | LCD_ENABLE_HIGH | ((addr & 0x0F) << 4);
    tx_packet->buffer[3] = cmd | LCD_ENABLE_LOW  | ((addr & 0x0F) << 4);
    
    // write string ...
    cmd = LCD_WRITE_DDR | backlight;
    for(count = 0; count < strlen(str); count++)
    {
        byte = str[count];
        tx_packet->buffer[4 + count * 4 + 0] = cmd | LCD_ENABLE_HIGH | (byte & 0xF0);
        tx_packet->buffer[4 + count * 4 + 1] = cmd | LCD_ENABLE_LOW  | (byte & 0xF0);
        tx_packet->buffer[4 + count * 4 + 2] = cmd | LCD_ENABLE_HIGH | ((byte & 0x0F) << 4);
        tx_packet->buffer[4 + count * 4 + 3] = cmd | LCD_ENABLE_LOW  | ((byte & 0x0F) << 4);
    }
    tx_packet->length = 4 + count * 4;

    lcdIsBusy = true;
    twi_pdc_master_write(TWI0, tx_packet);
    lcdStartTicks = getTicks();
}

void lcd_print_m(const char* str[])
{
    twi_packet_t *tx_packet = twi_get_tx_packet();
    uint8_t addr;
    uint8_t cmd;
    uint8_t byte;
    uint8_t count;
    uint8_t line;
    uint16_t index = 0;
    
    if(!lcd_in_use)
    {
        return;
    }
    
    lcd_clear(); // added 03.03.19 by Marc
    
    while(lcdIsBusy | twi_is_busy())
    {
        if((getTicks() - lcdStartTicks) > LCD_TIMEOUT_DELAY)
        {
            lcdIsBusy = false;
            lcdTimeoutErrorCntr++;
            pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS | PERIPH_PTCR_RXTDIS);
            twi_master_setup(TWI0, &twiConfig);
            break;
        }
    }

    for(line = 0; line < 4; line++)
    {
        // Calculate display address
        switch(line)
        {
            case 1:
            addr = 0x80 | 0x40;   // set address to start of line 2
            break;
            case 2:
            addr = 0x80 | 0x14;   // set address to start of line 3
            break;
            case 3:
            addr = 0x80 | 0x54;   // set address to start of line 4
            break;
            case 0:
            default:
            addr = 0x80;          // set address to start of line 1
            break;
        }

        // Set cursor
        cmd = LCD_FUNTION_WRITE | backlight;
        tx_packet->buffer[index++] = cmd | LCD_ENABLE_HIGH | (addr & 0xF0);
        tx_packet->buffer[index++] = cmd | LCD_ENABLE_LOW  | (addr & 0xF0);
        tx_packet->buffer[index++] = cmd | LCD_ENABLE_HIGH | ((addr & 0x0F) << 4);
        tx_packet->buffer[index++] = cmd | LCD_ENABLE_LOW  | ((addr & 0x0F) << 4);
        
        // write string ...
        cmd = LCD_WRITE_DDR | backlight;
        for(count = 0; count < strlen(str[line]); count++)
        {
            byte = str[line][count];
            tx_packet->buffer[index++] = cmd | LCD_ENABLE_HIGH | (byte & 0xF0);
            tx_packet->buffer[index++] = cmd | LCD_ENABLE_LOW  | (byte & 0xF0);
            tx_packet->buffer[index++] = cmd | LCD_ENABLE_HIGH | ((byte & 0x0F) << 4);
            tx_packet->buffer[index++] = cmd | LCD_ENABLE_LOW  | ((byte & 0x0F) << 4);
        }
    }
    tx_packet->length = index;

    lcdIsBusy = true;
    twi_pdc_master_write(TWI0, tx_packet);
    lcdStartTicks = getTicks();
}

uint8_t lcd_is_busy(void)
{
    return lcdIsBusy;
}

uint32_t lcd_get_timeout_error_cntr(void)
{
    return lcdTimeoutErrorCntr;
}

/************************************************************
* Local Functions                                           *
************************************************************/
static void send_nibble(uint8_t cmd, uint8_t byte)
{
    twi_packet_t *tx_packet = twi_get_tx_packet();
    
    if(!lcd_in_use)
    {
        return;
    }

    while(lcdIsBusy | twi_is_busy())
    {
        if((getTicks() - lcdStartTicks) > LCD_TIMEOUT_DELAY)
        {
            lcdIsBusy = false;
            lcdTimeoutErrorCntr++;
            pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS | PERIPH_PTCR_RXTDIS);
            twi_master_setup(TWI0, &twiConfig);
            break;
        }
    }

    tx_packet->chip = 0x27;
    tx_packet->addr[0] = 0x00;
    tx_packet->addr_length = 0;
    
    cmd = (cmd & 0x0F) | backlight;                  /* mask command bits */
    tx_packet->buffer[0] = cmd | LCD_ENABLE_HIGH | ((byte & 0x0F) << 4);
    tx_packet->buffer[1] = cmd | LCD_ENABLE_LOW  | ((byte & 0x0F) << 4);
    tx_packet->length = 2;
    
    lcdIsBusy = true;
    twi_pdc_master_write(TWI0, tx_packet);
    lcdStartTicks = getTicks();
}

static void send_byte(uint8_t cmd, uint8_t byte)
{
    twi_packet_t *tx_packet = twi_get_tx_packet();

    if(!lcd_in_use)
    {
        return;
    }

    while(lcdIsBusy | twi_is_busy())
    {
        if((getTicks() - lcdStartTicks) > LCD_TIMEOUT_DELAY)
        {
            lcdIsBusy = false;
            lcdTimeoutErrorCntr++;
            pdc_disable_transfer(PDC_TWI0, PERIPH_PTCR_TXTDIS | PERIPH_PTCR_RXTDIS);
            twi_master_setup(TWI0, &twiConfig);
            break;
        }
    }

    tx_packet->chip = 0x27;
    tx_packet->addr[0] = 0x00;
    tx_packet->addr_length = 0;
    
    cmd = (cmd & 0x0F) | backlight;                  /* mask command bits */
    tx_packet->buffer[0] = cmd | LCD_ENABLE_HIGH | (byte & 0xF0) ;
    tx_packet->buffer[1] = cmd | LCD_ENABLE_LOW  | (byte & 0xF0) ;
    tx_packet->buffer[2] = cmd | LCD_ENABLE_HIGH | ((byte & 0x0F) << 4);
    tx_packet->buffer[3] = cmd | LCD_ENABLE_LOW  | ((byte & 0x0F) << 4);
    tx_packet->length = 4;
    
    lcdIsBusy = true;
    twi_pdc_master_write(TWI0, tx_packet);
    lcdStartTicks = getTicks();
}

static void lcd_callback(void)
{
    lcdIsBusy = false;
}
