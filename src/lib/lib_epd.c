/***************************************************************************************************
 *
 * @file    lib_epd.c
 * @brief   The library of waveshare 4.3 inch E-Paper screen.
 *
 *          Frame format:
 *          --------------------------------------------------------------------
 *          | Start|   Length  |  Cmd |  Data  |         End         | Checksum|
 *          |      |Start~Check|      | 0~1024 |                     | XOR S~E |
 *          --------------------------------------------------------------------
 *          | 0xA5 | 0x00 0x00 | 0x00 |        | 0xCC 0x33 0xC3 0x3C |   0x00  |
 *          --------------------------------------------------------------------
 * 
 *          So frames without data contains 0x0009 bytes.
 *
 * @author  amaruk@163.com
 * @date    2017/02/26
 *
 **************************************************************************************************/

#include "common.h"
#include "lib_epd.h"
#include "drv_uart.h"

static int wake_up = 0; /* Wake up pin */
static int reset = 0; /* Reset pin */

/* Command frames */
static const unsigned char _frame_handshake[8] =
{ START, 0x00, 0x09, CMD_HANDSHAKE, END_0, END_1, END_2, END_3 };     //CMD_HANDSHAKE
static const unsigned char _frame_read_baud[8] =
{ START, 0x00, 0x09, CMD_READ_BAUD, END_0, END_1, END_2, END_3 };     //CMD_READ_BAUD
static const unsigned char _frame_stopmode[8] =
{ START, 0x00, 0x09, CMD_STOP_MODE, END_0, END_1, END_2, END_3 };       //CMD_STOPMODE
static const unsigned char _frame_update[8] =
{ START, 0x00, 0x09, CMD_UPDATE, END_0, END_1, END_2, END_3 };           //CMD_UPDATE
static const unsigned char _frame_load_font[8] =
{ START, 0x00, 0x09, CMD_LOAD_FONT, END_0, END_1, END_2, END_3 };     //CMD_LOAD_FONT
static const unsigned char _frame_load_pic[8] =
{ START, 0x00, 0x09, CMD_LOAD_PIC, END_0, END_1, END_2, END_3 };       //CMD_LOAD_PIC
static const unsigned char _frame_byte[9] =                             //Cmd with byte data
{ START, 0x00, 0x09, CMD_LOAD_PIC, CMD_DATA_BYTE, END_0, END_1, END_2, END_3 };
static const unsigned char _frame_short[10] =                           //Cmd with short data
{ START, 0x00, 0x09, CMD_LOAD_PIC, CMD_DATA_BYTE, CMD_DATA_BYTE, END_0, END_1, END_2, END_3 };
static const unsigned char _frame_dword[12] =                           //Cmd with dword data
{ START, 0x00, 0x09, CMD_LOAD_PIC, CMD_DATA_BYTE, CMD_DATA_BYTE, CMD_DATA_BYTE, CMD_DATA_BYTE, END_0, END_1, END_2, END_3 };

/* Command data */
static unsigned char _frame_buff[FRAME_BUFF_SIZE];

/* Generate checksum */
static unsigned char _checksum(const void * ptr, int n)
{
    int i;
    unsigned char * p = (unsigned char *) ptr;
    unsigned char result;

    for (i = 0, result = 0; i < n; i++)
    {
        result ^= p[i];
    }

    return result;
}

#define SYSFS_UART_DEV "/sys/devices/bone_capemgr.9/slots"

/* Initialization */
void lib_epd_init(void)
{
#if defined(PLATFORM_BBB)
    FILE *fd = NULL;

    fd = fopen(SYSFS_UART_DEV, "w");
    fwrite("BB-UART4", sizeof(int), 8, fd); /* "BB-UART4" length is 8 */
    fclose(fd);

    drv_uart_init("/dev/ttyO4", 115200, 8, 1, 'N');
#elif defined(PLATFORM_UBUNTU)
    drv_uart_init("/dev/ttyUSB0", 115200, 8, 1, 'N');
#elif defined(PLATFORM_CYGWIN)
    drv_uart_init("/dev/ttyS3", 115200, 8, 1, 'N');
#endif
    wake_up = PIN_LOW;
    reset = PIN_LOW;
}

/* Close communication with the e-paper */
void lib_epd_close(void)
{
    drv_uart_kill();
}

/* Use the reset pin to reset the e-paper */
void lib_epd_reset(void)
{
    reset = 0;
    usleep(10);
    reset = 1;
    usleep(500);
    reset = 0;
    usleep(3000000);
}

/* Wake up the e-paper */
void lib_epd_wakeup(void)
{
    wake_up = PIN_LOW;
    usleep(10);
    wake_up = PIN_HIGH;
    usleep(500);
    wake_up = PIN_LOW;
    usleep(10);
}

/* Handshake */
void lib_epd_handshake(void)
{
    memcpy(_frame_buff, _frame_handshake, 8);
    _frame_buff[8] = _checksum(_frame_buff, 8);

    drv_uart_putchars(_frame_buff, 9);
    drv_uart_getchars(_frame_buff); // Returns "OK" if epaper is ready
    // TODO: Check handshake result
}

/* Set baudrate */
void lib_epd_set_baud(long baud)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x0D;

    _frame_buff[3] = CMD_SET_BAUD;

    _frame_buff[4] = (baud >> 24) & 0xFF;
    _frame_buff[5] = (baud >> 16) & 0xFF;
    _frame_buff[6] = (baud >> 8) & 0xFF;
    _frame_buff[7] = baud & 0xFF;

    _frame_buff[8] = END_0;
    _frame_buff[9] = END_1;
    _frame_buff[10] = END_2;
    _frame_buff[11] = END_3;
    _frame_buff[12] = _checksum(_frame_buff, 12);

    drv_uart_putchars(_frame_buff, 13);

    usleep(10000);
}

/* Read baudrate */
void lib_epd_read_baud(void)
{
    memcpy(_frame_buff, _frame_read_baud, 8);
    _frame_buff[8] = _checksum(_frame_buff, 8);

    drv_uart_putchars(_frame_buff, 9);
    // TODO: Read baud in ASCII format
}

/* Choose memory to be used.
 * mode: MEM_TF(1) or MEM_NAND(0) */
void lib_epd_set_memory(unsigned char mode)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x0A;

    _frame_buff[3] = CMD_SET_MEM_MODE;

    _frame_buff[4] = mode;

    _frame_buff[5] = END_0;
    _frame_buff[6] = END_1;
    _frame_buff[7] = END_2;
    _frame_buff[8] = END_3;
    _frame_buff[9] = _checksum(_frame_buff, 9);

    drv_uart_putchars(_frame_buff, 10);
}

/* Enter stop mode */
void lib_epd_enter_stopmode(void)
{
    memcpy(_frame_buff, _frame_stopmode, 8);
    _frame_buff[8] = _checksum(_frame_buff, 8);

    drv_uart_putchars(_frame_buff, 9);
}

/* Update the e-paper's screen:
 * Flush buffer to screen.
 */
void lib_epd_udpate(void)
{
    memcpy(_frame_buff, _frame_update, 8);
    _frame_buff[8] = _checksum(_frame_buff, 8);

    drv_uart_putchars(_frame_buff, 9);
}

/* Normal screen (0) or upside down screen (1) */
void lib_epd_screen_rotation(unsigned char mode)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x0A;

    _frame_buff[3] = CMD_SET_SCR_ROTATION;

    _frame_buff[4] = mode;

    _frame_buff[5] = END_0;
    _frame_buff[6] = END_1;
    _frame_buff[7] = END_2;
    _frame_buff[8] = END_3;
    _frame_buff[9] = _checksum(_frame_buff, 9);

    drv_uart_putchars(_frame_buff, 10);
}

/* Load font from TF to NAND */
void lib_epd_load_font(void)
{
    memcpy(_frame_buff, _frame_load_font, 8);
    _frame_buff[8] = _checksum(_frame_buff, 8);

    drv_uart_putchars(_frame_buff, 9);
}

/* Load BMP from TF to NAND */
void lib_epd_load_pic(void)
{
    memcpy(_frame_buff, _frame_load_pic, 8);
    _frame_buff[8] = _checksum(_frame_buff, 8);

    drv_uart_putchars(_frame_buff, 9);
}

/* Set fore-ground and back-ground colours */
void lib_epd_set_color(unsigned char color, unsigned char bkcolor)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x0B;

    _frame_buff[3] = CMD_SET_COLOR;

    _frame_buff[4] = color; // Foreground
    _frame_buff[5] = bkcolor; // Background

    _frame_buff[6] = END_0;
    _frame_buff[7] = END_1;
    _frame_buff[8] = END_2;
    _frame_buff[9] = END_3;
    _frame_buff[10] = _checksum(_frame_buff, 10);

    drv_uart_putchars(_frame_buff, 11);
}

/* Set English font: 1:32dot 2:48dot 3:64dot */
void lib_epd_set_en_font(unsigned char font)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x0A;

    _frame_buff[3] = CMD_SET_EN_FONT;

    _frame_buff[4] = font;

    _frame_buff[5] = END_0;
    _frame_buff[6] = END_1;
    _frame_buff[7] = END_2;
    _frame_buff[8] = END_3;
    _frame_buff[9] = _checksum(_frame_buff, 9);

    drv_uart_putchars(_frame_buff, 10);
}

/* Set Chinese font: 1:32dot 2:48dot 3:64dot */
void lib_epd_set_ch_font(unsigned char font)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x0A;

    _frame_buff[3] = CMD_SET_CH_FONT;

    _frame_buff[4] = font;

    _frame_buff[5] = END_0;
    _frame_buff[6] = END_1;
    _frame_buff[7] = END_2;
    _frame_buff[8] = END_3;
    _frame_buff[9] = _checksum(_frame_buff, 9);

    drv_uart_putchars(_frame_buff, 10);
}

/* Draw single pixel */
// TODO: should be int16
void lib_epd_draw_pixel(int x0, int y0)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x0D;

    _frame_buff[3] = CMD_DRAW_PIXEL;

    _frame_buff[4] = (x0 >> 8) & 0xFF;
    _frame_buff[5] = x0 & 0xFF;
    _frame_buff[6] = (y0 >> 8) & 0xFF;
    _frame_buff[7] = y0 & 0xFF;

    _frame_buff[8] = END_0;
    _frame_buff[9] = END_1;
    _frame_buff[10] = END_2;
    _frame_buff[11] = END_3;
    _frame_buff[12] = _checksum(_frame_buff, 12);

    drv_uart_putchars(_frame_buff, 13);
}

/* Draw line */
// TODO: Should be int16
void lib_epd_draw_line(int x0, int y0, int x1, int y1)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x11;

    _frame_buff[3] = CMD_DRAW_LINE;

    _frame_buff[4] = (x0 >> 8) & 0xFF;
    _frame_buff[5] = x0 & 0xFF;
    _frame_buff[6] = (y0 >> 8) & 0xFF;
    _frame_buff[7] = y0 & 0xFF;
    _frame_buff[8] = (x1 >> 8) & 0xFF;
    _frame_buff[9] = x1 & 0xFF;
    _frame_buff[10] = (y1 >> 8) & 0xFF;
    _frame_buff[11] = y1 & 0xFF;

    _frame_buff[12] = END_0;
    _frame_buff[13] = END_1;
    _frame_buff[14] = END_2;
    _frame_buff[15] = END_3;
    _frame_buff[16] = _checksum(_frame_buff, 16);

    drv_uart_putchars(_frame_buff, 17);
}

/* Fill rectangle */
// TODO: should be int16
void lib_epd_fill_rect(int x0, int y0, int x1, int y1)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x11;

    _frame_buff[3] = CMD_FILL_RECT;

    _frame_buff[4] = (x0 >> 8) & 0xFF;
    _frame_buff[5] = x0 & 0xFF;
    _frame_buff[6] = (y0 >> 8) & 0xFF;
    _frame_buff[7] = y0 & 0xFF;
    _frame_buff[8] = (x1 >> 8) & 0xFF;
    _frame_buff[9] = x1 & 0xFF;
    _frame_buff[10] = (y1 >> 8) & 0xFF;
    _frame_buff[11] = y1 & 0xFF;

    _frame_buff[12] = END_0;
    _frame_buff[13] = END_1;
    _frame_buff[14] = END_2;
    _frame_buff[15] = END_3;
    _frame_buff[16] = _checksum(_frame_buff, 16);

    drv_uart_putchars(_frame_buff, 17);
}

/* Draw circle */
// TODO: should be int16
void lib_epd_draw_circle(int x0, int y0, int r)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x0F;

    _frame_buff[3] = CMD_DRAW_CIRCLE;

    _frame_buff[4] = (x0 >> 8) & 0xFF;
    _frame_buff[5] = x0 & 0xFF;
    _frame_buff[6] = (y0 >> 8) & 0xFF;
    _frame_buff[7] = y0 & 0xFF;
    _frame_buff[8] = (r >> 8) & 0xFF;
    _frame_buff[9] = r & 0xFF;

    _frame_buff[10] = END_0;
    _frame_buff[11] = END_1;
    _frame_buff[12] = END_2;
    _frame_buff[13] = END_3;
    _frame_buff[14] = _checksum(_frame_buff, 14);

    drv_uart_putchars(_frame_buff, 15);
}

/* Fill circle */
// TODO: should be int16
void lib_epd_fill_circle(int x0, int y0, int r)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x0F;

    _frame_buff[3] = CMD_FILL_CIRCLE;

    _frame_buff[4] = (x0 >> 8) & 0xFF;
    _frame_buff[5] = x0 & 0xFF;
    _frame_buff[6] = (y0 >> 8) & 0xFF;
    _frame_buff[7] = y0 & 0xFF;
    _frame_buff[8] = (r >> 8) & 0xFF;
    _frame_buff[9] = r & 0xFF;

    _frame_buff[10] = END_0;
    _frame_buff[11] = END_1;
    _frame_buff[12] = END_2;
    _frame_buff[13] = END_3;
    _frame_buff[14] = _checksum(_frame_buff, 14);

    drv_uart_putchars(_frame_buff, 15);
}

/* Draw triangle */
// TODO: should be int16
void lib_epd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x15;

    _frame_buff[3] = CMD_DRAW_TRIANGLE;

    _frame_buff[4] = (x0 >> 8) & 0xFF;
    _frame_buff[5] = x0 & 0xFF;
    _frame_buff[6] = (y0 >> 8) & 0xFF;
    _frame_buff[7] = y0 & 0xFF;
    _frame_buff[8] = (x1 >> 8) & 0xFF;
    _frame_buff[9] = x1 & 0xFF;
    _frame_buff[10] = (y1 >> 8) & 0xFF;
    _frame_buff[11] = y1 & 0xFF;
    _frame_buff[12] = (x2 >> 8) & 0xFF;
    _frame_buff[13] = x2 & 0xFF;
    _frame_buff[14] = (y2 >> 8) & 0xFF;
    _frame_buff[15] = y2 & 0xFF;

    _frame_buff[16] = END_0;
    _frame_buff[17] = END_1;
    _frame_buff[18] = END_2;
    _frame_buff[19] = END_3;
    _frame_buff[20] = _checksum(_frame_buff, 20);

    drv_uart_putchars(_frame_buff, 21);
}

/* Fill triangle */
// TODO: should be int16
void lib_epd_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x15;

    _frame_buff[3] = CMD_FILL_TRIANGLE;

    _frame_buff[4] = (x0 >> 8) & 0xFF;
    _frame_buff[5] = x0 & 0xFF;
    _frame_buff[6] = (y0 >> 8) & 0xFF;
    _frame_buff[7] = y0 & 0xFF;
    _frame_buff[8] = (x1 >> 8) & 0xFF;
    _frame_buff[9] = x1 & 0xFF;
    _frame_buff[10] = (y1 >> 8) & 0xFF;
    _frame_buff[11] = y1 & 0xFF;
    _frame_buff[12] = (x2 >> 8) & 0xFF;
    _frame_buff[13] = x2 & 0xFF;
    _frame_buff[14] = (y2 >> 8) & 0xFF;
    _frame_buff[15] = y2 & 0xFF;

    _frame_buff[16] = END_0;
    _frame_buff[17] = END_1;
    _frame_buff[18] = END_2;
    _frame_buff[19] = END_3;
    _frame_buff[20] = _checksum(_frame_buff, 20);

    drv_uart_putchars(_frame_buff, 21);
}

/* Clear screen using the background colour */
void lib_epd_clear(void)
{
    _frame_buff[0] = START;

    _frame_buff[1] = 0x00;
    _frame_buff[2] = 0x09;

    _frame_buff[3] = CMD_CLEAR;

    _frame_buff[4] = END_0;
    _frame_buff[5] = END_1;
    _frame_buff[6] = END_2;
    _frame_buff[7] = END_3;
    _frame_buff[8] = _checksum(_frame_buff, 8);

    drv_uart_putchars(_frame_buff, 9);
}

/* Display a single char */
void lib_epd_disp_char(unsigned char ch, int x0, int y0)
{
    unsigned char buff[2];

    buff[0] = ch;
    buff[1] = 0;

    lib_epd_disp_string(buff, x0, y0);
}

/* Display text */
// TODO: Fix the buff size bug
// TODO: should be int16
void lib_epd_disp_string(const void * p, int x0, int y0)
{
    int string_size;
    unsigned char * ptr = (unsigned char *) p;

    string_size = strlen((const char *) ptr);
    string_size += 14;

    _frame_buff[0] = START;

    _frame_buff[1] = (string_size >> 8) & 0xFF;
    _frame_buff[2] = string_size & 0xFF;

    _frame_buff[3] = CMD_DRAW_STRING;

    _frame_buff[4] = (x0 >> 8) & 0xFF;
    _frame_buff[5] = x0 & 0xFF;
    _frame_buff[6] = (y0 >> 8) & 0xFF;
    _frame_buff[7] = y0 & 0xFF;

    strcpy((char *) (&_frame_buff[8]), (const char *) ptr);

    string_size -= 5;

    _frame_buff[string_size] = END_0;
    _frame_buff[string_size + 1] = END_1;
    _frame_buff[string_size + 2] = END_2;
    _frame_buff[string_size + 3] = END_3;
    _frame_buff[string_size + 4] = _checksum(_frame_buff, string_size + 4);

    drv_uart_putchars(_frame_buff, string_size + 5);
}

/* Display BMP. Bitmap file name string maximum length is 11 */
// TODO: should be int16
void lib_epd_disp_bitmap(const void * p, int x0, int y0)
{
    int string_size;
    unsigned char * ptr = (unsigned char *) p;

    string_size = strlen((const char *) ptr);
    string_size += 14;

    _frame_buff[0] = START;

    _frame_buff[1] = (string_size >> 8) & 0xFF;
    _frame_buff[2] = string_size & 0xFF;

    _frame_buff[3] = CMD_DRAW_BITMAP;

    _frame_buff[4] = (x0 >> 8) & 0xFF;
    _frame_buff[5] = x0 & 0xFF;
    _frame_buff[6] = (y0 >> 8) & 0xFF;
    _frame_buff[7] = y0 & 0xFF;

    strcpy((char *) (&_frame_buff[8]), (const char *) ptr);

    string_size -= 5;

    _frame_buff[string_size] = END_0;
    _frame_buff[string_size + 1] = END_1;
    _frame_buff[string_size + 2] = END_2;
    _frame_buff[string_size + 3] = END_3;
    _frame_buff[string_size + 4] = _checksum(_frame_buff, string_size + 4);

    drv_uart_putchars(_frame_buff, string_size + 5);
}

