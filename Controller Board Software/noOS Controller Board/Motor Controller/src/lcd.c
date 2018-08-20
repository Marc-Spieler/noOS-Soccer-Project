/************************************************************************/
/* Author: Marc Spieler                                                 */
/* Team: noOS                                                           */
/* Created: 29.01.18                                                    */
/************************************************************************/

#include "lcd.h"
#include "timing.h"
#include "string.h"

#define LCD_ENABLE_HIGH       0x04
#define LCD_ENABLE_LOW        0x00
#define LCD_WRITE_DDR         0x01
#define LCD_FUNTION_WRITE     0x00

#define LCD_CLEAR_WAIT_TIME   10                         /* ms  */

static backlight_t backlight;
static twi_package_t tx_packet;

char lcd_line_1[20] = "                    ";
char lcd_line_2[20] = "                    ";
char lcd_line_3[20] = "                    ";
char lcd_line_4[20] = "                    ";

static void send_nibble(uint8_t cmd, uint8_t byte);
static void send_byte(uint8_t cmd, uint8_t byte);

void lcd_init(void)
{
    backlight = LCD_LIGHT_OFF;

    send_nibble(LCD_FUNTION_WRITE, 3);
    mdelay(5);
    send_nibble(LCD_FUNTION_WRITE, 3);
    mdelay(5);
    send_nibble(LCD_FUNTION_WRITE, 3);
    mdelay(5);
    send_nibble(LCD_FUNTION_WRITE, 2);                /* Interface auf 4-Bit setzen        */
    /*-----------------------------------*/
    send_byte(LCD_FUNTION_WRITE, 0x28);               /* Funktion Set:                     */
    /*   - Interface: 4-Bit              */
    /*   - LCD: 2-zeilig                 */
    /*   - Darstellung: 5x8-Punkte       */
    /*-----------------------------------*/
    send_byte(LCD_FUNTION_WRITE, 0x08);               /* Display ein/aus:                  */
    /*   - Display: aus                  */
    /*   - Kursor: aus                   */
    /*   - Kursor als Untersrtich        */
    /*-----------------------------------*/
    send_byte(LCD_FUNTION_WRITE, 0x01);               /* Display löschen                   */
    mdelay(LCD_CLEAR_WAIT_TIME);                      /*-----------------------------------*/
    send_byte(LCD_FUNTION_WRITE, 0x06);               /* Entry Mode Set:                   */
    /*   - Kuror Laufrichtung: rechts    */
    /*   - kein Shift                    */
    /*-----------------------------------*/
    send_byte(LCD_FUNTION_WRITE, 0x0C);               /* Display ein/aus:                  */
    /*   - Display: ein                  */
    /*   - Kursor: aus                   */
    /*   - Kursor als Untersrtich        */
}

void set_backlight(backlight_t state)
{
    backlight = state;
    send_byte(LCD_FUNTION_WRITE, 0x01);               /* clear dislplay */
    mdelay(LCD_CLEAR_WAIT_TIME);
}

void lcd_clear(void)
{
    send_byte(0, 0x01);
    mdelay(LCD_CLEAR_WAIT_TIME);
}

void print_s(uint8_t line, uint8_t col, const char* data)
{
    int len=strlen(data);
    uint8_t addr;
	
    if (line > 4 || line < 1)
    {
        return;
    }
	
    switch (line)
    {
        case 1:
            addr = 0x80;
            break;
        case 2:
            addr = 0x80 | 0x40;                             /* set address to start of line 2           */
            break;
        case 3:
            addr = 0x80 | 0x14;                             /* set address to start of line 3           */
            break;
        case 4:
            addr = 0x80 | 0x54;                             /* set address to start of line 4           */
            break;
        default:
            addr = 0x80;
            break;
    }
	
    addr += col;                                        /* set column in the selected  line         */
    send_byte(LCD_FUNTION_WRITE, addr);               /* set cursor to the address                */
	
    for (int i=0; i<len; i++)
    {                         /* write sting.....                         */
        send_byte(LCD_WRITE_DDR, data[i]);
    }
}

void lcd_print_c(int data)
{
    int temp = data;
    int k;
    uint8_t temp_buf[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', 0 };
    uint8_t temp_sign;
    
    if (data < 0)
    {
        temp_sign = '-';
        temp = ~((int)data - (int)1);
    }
    else
    {
        temp_sign = ' ';
    }
    
    k = 0;
    
    do
    {
        temp_buf[k] = (uint8_t)('0' + (temp % 10));
        k++;
        temp /= 10;
    } while (temp);
    
    temp_buf[k] = temp_sign;
    k = 6;
    
    while (k>=0)
    {
        send_byte(LCD_WRITE_DDR, temp_buf[k]);
        k--;
    }
}

void print_c(uint8_t line, uint8_t col, int data)
{
    uint8_t addr;
	
    switch (line)
    {
        case 1:
            addr = 0x80;
            break;
        case 2:
            addr = 0x80 | 0x40;                         /* set address to start of line 2           */
            break;
        case 3:
            addr = 0x80 | 0x14;                         /* set address to start of line 3           */
            break;
        case 4:
            addr = 0x80 | 0x54;                         /* set address to start of line 4           */
            break;
        default:
            addr = 0x80;
            break;
    }
	
    addr += col;                                      /* set column in the selected  line         */
    send_byte(LCD_FUNTION_WRITE, addr);             /* set cursor to the address                */
    lcd_print_c(data);
}

static void send_nibble(uint8_t cmd, uint8_t byte)
{
    uint8_t data[2];
	
    cmd = (cmd & 0x0F) | backlight;                  /* mask command bits */
    data[0] = cmd | LCD_ENABLE_HIGH | ((byte & 0x0F) << 4);
    data[1] = cmd | LCD_ENABLE_LOW  | ((byte & 0x0F) << 4);

    tx_packet.chip = 0x27;
    tx_packet.addr[0] = 0x00;
    tx_packet.addr_length = 0;
    tx_packet.buffer = data;
    tx_packet.length = 2;
	
    while (twi_master_write(TWI0, &tx_packet) != TWI_SUCCESS);
}

static void send_byte(uint8_t cmd, uint8_t byte)
{
    uint8_t data[4];
	
    cmd = (cmd & 0x0F) | backlight;                  /* mask command bits */
    data[0] = cmd | LCD_ENABLE_HIGH | (byte & 0xF0) ;
    data[1] = cmd | LCD_ENABLE_LOW  | (byte & 0xF0) ;
    data[2] = cmd | LCD_ENABLE_HIGH | ((byte & 0x0F) << 4);
    data[3] = cmd | LCD_ENABLE_LOW  | ((byte & 0x0F) << 4);
	
    tx_packet.chip = 0x27;
    tx_packet.addr[0] = 0x00;
    tx_packet.addr_length = 0;
    tx_packet.buffer = data;
    tx_packet.length = 4;
	
    while (twi_master_write(TWI0, &tx_packet) != TWI_SUCCESS);
}

void twi_init(void)
{
    twi_master_options_t opt =
    {
        .speed = 100000,
        .chip  = 0x00
    };
    
    twi_master_setup(TWI0, &opt);
}

/*void update_lcd(void)
{
    lcd_line_3[5] = '+';
    
    if (getTicks() >= (ticks_lcd_update + 1000))
    {
        ticks_lcd_update = getTicks();
        print_s(1, 0, lcd_line_1);
        print_s(2, 0, lcd_line_2);
        print_s(3, 0, lcd_line_3);
        print_s(4, 0, lcd_line_4);
    }
}*/
