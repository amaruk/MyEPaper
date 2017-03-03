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

/* The following pins are not in use now */
static int s_pin_wakeup = 0;    /* Wake up pin */
static int s_pin_reset = 0;     /* Reset pin */

/* Command frames */
static const unsigned char s_frame_handshake[8] =
{ START, 0x00, 0x09, CMD_HANDSHAKE, END_0, END_1, END_2, END_3 };     //CMD_HANDSHAKE
static const unsigned char s_frame_read_baud[8] =
{ START, 0x00, 0x09, CMD_READ_BAUD, END_0, END_1, END_2, END_3 };     //CMD_READ_BAUD
static const unsigned char s_frame_stopmode[8] =
{ START, 0x00, 0x09, CMD_STOP_MODE, END_0, END_1, END_2, END_3 };       //CMD_STOPMODE
static const unsigned char s_frame_update[8] =
{ START, 0x00, 0x09, CMD_UPDATE, END_0, END_1, END_2, END_3 };           //CMD_UPDATE
static const unsigned char s_frame_load_font[8] =
{ START, 0x00, 0x09, CMD_LOAD_FONT, END_0, END_1, END_2, END_3 };     //CMD_LOAD_FONT
static const unsigned char s_frame_load_pic[8] =
{ START, 0x00, 0x09, CMD_LOAD_PIC, END_0, END_1, END_2, END_3 };       //CMD_LOAD_PIC
static const unsigned char s_frame_byte[9] =                             //Cmd with byte data
{ START, 0x00, 0x09, CMD_LOAD_PIC, CMD_DATA_BYTE, END_0, END_1, END_2, END_3 };
static const unsigned char s_frame_short[10] =                           //Cmd with short data
{ START, 0x00, 0x09, CMD_LOAD_PIC, CMD_DATA_BYTE, CMD_DATA_BYTE, END_0, END_1, END_2, END_3 };
static const unsigned char s_frame_dword[12] =                           //Cmd with dword data
{ START, 0x00, 0x09, CMD_LOAD_PIC, CMD_DATA_BYTE, CMD_DATA_BYTE, CMD_DATA_BYTE, CMD_DATA_BYTE, END_0, END_1, END_2, END_3 };

/* Command data */
static unsigned char s_frame_buff[FRAME_BUFF_SIZE];

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
void LibEpdInit(void)
{
#if defined(PLATFORM_BBB)
    FILE *fd = NULL;

    fd = fopen(SYSFS_UART_DEV, "w");
    fwrite("BB-UART4", sizeof(int), 8, fd); /* "BB-UART4" length is 8 */
    fclose(fd);

    DrvUartInit("/dev/ttyO4", 115200, 8, 1, 'N');
#elif defined(PLATFORM_UBUNTU)
    DrvUartInit("/dev/ttyUSB0", 115200, 8, 1, 'N');
#elif defined(PLATFORM_CYGWIN)
    DrvUartInit("/dev/ttyS35", 115200, 8, 1, 'N');
#endif
    s_pin_wakeup = PIN_LOW;
    s_pin_reset = PIN_LOW;
}

/* Close communication with the e-paper */
void LibEpdClose(void)
{
    DrvUartKill();
}

/* Use the reset pin to reset the e-paper */
void LibEpdReset(void)
{
    s_pin_reset = 0;
    usleep(10);
    s_pin_reset = 1;
    usleep(500);
    s_pin_reset = 0;
    usleep(3000000);
}

/* Wake up the e-paper */
void LibEpdWakeup(void)
{
    s_pin_wakeup = PIN_LOW;
    usleep(10);
    s_pin_wakeup = PIN_HIGH;
    usleep(500);
    s_pin_wakeup = PIN_LOW;
    usleep(10);
}

/* Handshake */
void LibEpdHandshake(void)
{
    memcpy(s_frame_buff, s_frame_handshake, 8);
    s_frame_buff[8] = _checksum(s_frame_buff, 8);

    DrvUartPutchars(s_frame_buff, 9);
    DrvUartGetChars(s_frame_buff); // Returns "OK" if epaper is ready
    // TODO: Check handshake result
}

/* Set baudrate */
void LibEpdSetBaud(long baud)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x0D;

    s_frame_buff[3] = CMD_SET_BAUD;

    s_frame_buff[4] = (baud >> 24) & 0xFF;
    s_frame_buff[5] = (baud >> 16) & 0xFF;
    s_frame_buff[6] = (baud >> 8) & 0xFF;
    s_frame_buff[7] = baud & 0xFF;

    s_frame_buff[8] = END_0;
    s_frame_buff[9] = END_1;
    s_frame_buff[10] = END_2;
    s_frame_buff[11] = END_3;
    s_frame_buff[12] = _checksum(s_frame_buff, 12);

    DrvUartPutchars(s_frame_buff, 13);

    usleep(10000);
}

/* Read baudrate */
void LibEpdReadBaud(void)
{
    memcpy(s_frame_buff, s_frame_read_baud, 8);
    s_frame_buff[8] = _checksum(s_frame_buff, 8);

    DrvUartPutchars(s_frame_buff, 9);
    // TODO: Read baud in ASCII format
}

/* Choose memory to be used.
 * mode: MEM_TF(1) or MEM_NAND(0) */
void LibEpdSetMemory(unsigned char mode)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x0A;

    s_frame_buff[3] = CMD_SET_MEM_MODE;

    s_frame_buff[4] = mode;

    s_frame_buff[5] = END_0;
    s_frame_buff[6] = END_1;
    s_frame_buff[7] = END_2;
    s_frame_buff[8] = END_3;
    s_frame_buff[9] = _checksum(s_frame_buff, 9);

    DrvUartPutchars(s_frame_buff, 10);
}

/* Enter stop mode */
void LibEpdEnterStopMode(void)
{
    memcpy(s_frame_buff, s_frame_stopmode, 8);
    s_frame_buff[8] = _checksum(s_frame_buff, 8);

    DrvUartPutchars(s_frame_buff, 9);
}

/* Update the e-paper's screen:
 * Flush buffer to screen.
 */
void LibEpdUpdate(void)
{
    memcpy(s_frame_buff, s_frame_update, 8);
    s_frame_buff[8] = _checksum(s_frame_buff, 8);

    DrvUartPutchars(s_frame_buff, 9);
}

/* Normal screen (0) or upside down screen (1) */
void LibEpdScreenRotation(unsigned char mode)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x0A;

    s_frame_buff[3] = CMD_SET_SCR_ROTATION;

    s_frame_buff[4] = mode;

    s_frame_buff[5] = END_0;
    s_frame_buff[6] = END_1;
    s_frame_buff[7] = END_2;
    s_frame_buff[8] = END_3;
    s_frame_buff[9] = _checksum(s_frame_buff, 9);

    DrvUartPutchars(s_frame_buff, 10);
}

/* Load font from TF to NAND */
void LibEpdLoadFont(void)
{
    memcpy(s_frame_buff, s_frame_load_font, 8);
    s_frame_buff[8] = _checksum(s_frame_buff, 8);

    DrvUartPutchars(s_frame_buff, 9);
}

/* Load BMP from TF to NAND */
void LibEpdLoadPic(void)
{
    memcpy(s_frame_buff, s_frame_load_pic, 8);
    s_frame_buff[8] = _checksum(s_frame_buff, 8);

    DrvUartPutchars(s_frame_buff, 9);
}

/* Set fore-ground and back-ground colours */
void LibEpdSetColor(unsigned char color, unsigned char bkcolor)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x0B;

    s_frame_buff[3] = CMD_SET_COLOR;

    s_frame_buff[4] = color; // Foreground
    s_frame_buff[5] = bkcolor; // Background

    s_frame_buff[6] = END_0;
    s_frame_buff[7] = END_1;
    s_frame_buff[8] = END_2;
    s_frame_buff[9] = END_3;
    s_frame_buff[10] = _checksum(s_frame_buff, 10);

    DrvUartPutchars(s_frame_buff, 11);
}

/* Set English font: 1:32dot 2:48dot 3:64dot */
void LibEpdSetEnFont(unsigned char font)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x0A;

    s_frame_buff[3] = CMD_SET_EN_FONT;

    s_frame_buff[4] = font;

    s_frame_buff[5] = END_0;
    s_frame_buff[6] = END_1;
    s_frame_buff[7] = END_2;
    s_frame_buff[8] = END_3;
    s_frame_buff[9] = _checksum(s_frame_buff, 9);

    DrvUartPutchars(s_frame_buff, 10);
}

/* Set Chinese font: 1:32dot 2:48dot 3:64dot */
void LibEpdSetChFont(unsigned char font)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x0A;

    s_frame_buff[3] = CMD_SET_CH_FONT;

    s_frame_buff[4] = font;

    s_frame_buff[5] = END_0;
    s_frame_buff[6] = END_1;
    s_frame_buff[7] = END_2;
    s_frame_buff[8] = END_3;
    s_frame_buff[9] = _checksum(s_frame_buff, 9);

    DrvUartPutchars(s_frame_buff, 10);
}

/* Draw single pixel */
// TODO: should be int16
void LibEpdDrawPixel(int x0, int y0)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x0D;

    s_frame_buff[3] = CMD_DRAW_PIXEL;

    s_frame_buff[4] = (x0 >> 8) & 0xFF;
    s_frame_buff[5] = x0 & 0xFF;
    s_frame_buff[6] = (y0 >> 8) & 0xFF;
    s_frame_buff[7] = y0 & 0xFF;

    s_frame_buff[8] = END_0;
    s_frame_buff[9] = END_1;
    s_frame_buff[10] = END_2;
    s_frame_buff[11] = END_3;
    s_frame_buff[12] = _checksum(s_frame_buff, 12);

    DrvUartPutchars(s_frame_buff, 13);
}

/* Draw line */
// TODO: Should be int16
void LibEpdDrawLine(int x0, int y0, int x1, int y1)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x11;

    s_frame_buff[3] = CMD_DRAW_LINE;

    s_frame_buff[4] = (x0 >> 8) & 0xFF;
    s_frame_buff[5] = x0 & 0xFF;
    s_frame_buff[6] = (y0 >> 8) & 0xFF;
    s_frame_buff[7] = y0 & 0xFF;
    s_frame_buff[8] = (x1 >> 8) & 0xFF;
    s_frame_buff[9] = x1 & 0xFF;
    s_frame_buff[10] = (y1 >> 8) & 0xFF;
    s_frame_buff[11] = y1 & 0xFF;

    s_frame_buff[12] = END_0;
    s_frame_buff[13] = END_1;
    s_frame_buff[14] = END_2;
    s_frame_buff[15] = END_3;
    s_frame_buff[16] = _checksum(s_frame_buff, 16);

    DrvUartPutchars(s_frame_buff, 17);
}

/* Fill rectangle */
// TODO: should be int16
void LibEpdFillRect(int x0, int y0, int x1, int y1)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x11;

    s_frame_buff[3] = CMD_FILL_RECT;

    s_frame_buff[4] = (x0 >> 8) & 0xFF;
    s_frame_buff[5] = x0 & 0xFF;
    s_frame_buff[6] = (y0 >> 8) & 0xFF;
    s_frame_buff[7] = y0 & 0xFF;
    s_frame_buff[8] = (x1 >> 8) & 0xFF;
    s_frame_buff[9] = x1 & 0xFF;
    s_frame_buff[10] = (y1 >> 8) & 0xFF;
    s_frame_buff[11] = y1 & 0xFF;

    s_frame_buff[12] = END_0;
    s_frame_buff[13] = END_1;
    s_frame_buff[14] = END_2;
    s_frame_buff[15] = END_3;
    s_frame_buff[16] = _checksum(s_frame_buff, 16);

    DrvUartPutchars(s_frame_buff, 17);
}

/* Draw circle */
// TODO: should be int16
void LibEpdDrawCircle(int x0, int y0, int r)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x0F;

    s_frame_buff[3] = CMD_DRAW_CIRCLE;

    s_frame_buff[4] = (x0 >> 8) & 0xFF;
    s_frame_buff[5] = x0 & 0xFF;
    s_frame_buff[6] = (y0 >> 8) & 0xFF;
    s_frame_buff[7] = y0 & 0xFF;
    s_frame_buff[8] = (r >> 8) & 0xFF;
    s_frame_buff[9] = r & 0xFF;

    s_frame_buff[10] = END_0;
    s_frame_buff[11] = END_1;
    s_frame_buff[12] = END_2;
    s_frame_buff[13] = END_3;
    s_frame_buff[14] = _checksum(s_frame_buff, 14);

    DrvUartPutchars(s_frame_buff, 15);
}

/* Fill circle */
// TODO: should be int16
void LibEpdFillCircle(int x0, int y0, int r)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x0F;

    s_frame_buff[3] = CMD_FILL_CIRCLE;

    s_frame_buff[4] = (x0 >> 8) & 0xFF;
    s_frame_buff[5] = x0 & 0xFF;
    s_frame_buff[6] = (y0 >> 8) & 0xFF;
    s_frame_buff[7] = y0 & 0xFF;
    s_frame_buff[8] = (r >> 8) & 0xFF;
    s_frame_buff[9] = r & 0xFF;

    s_frame_buff[10] = END_0;
    s_frame_buff[11] = END_1;
    s_frame_buff[12] = END_2;
    s_frame_buff[13] = END_3;
    s_frame_buff[14] = _checksum(s_frame_buff, 14);

    DrvUartPutchars(s_frame_buff, 15);
}

/* Draw triangle */
// TODO: should be int16
void LibEpdDrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x15;

    s_frame_buff[3] = CMD_DRAW_TRIANGLE;

    s_frame_buff[4] = (x0 >> 8) & 0xFF;
    s_frame_buff[5] = x0 & 0xFF;
    s_frame_buff[6] = (y0 >> 8) & 0xFF;
    s_frame_buff[7] = y0 & 0xFF;
    s_frame_buff[8] = (x1 >> 8) & 0xFF;
    s_frame_buff[9] = x1 & 0xFF;
    s_frame_buff[10] = (y1 >> 8) & 0xFF;
    s_frame_buff[11] = y1 & 0xFF;
    s_frame_buff[12] = (x2 >> 8) & 0xFF;
    s_frame_buff[13] = x2 & 0xFF;
    s_frame_buff[14] = (y2 >> 8) & 0xFF;
    s_frame_buff[15] = y2 & 0xFF;

    s_frame_buff[16] = END_0;
    s_frame_buff[17] = END_1;
    s_frame_buff[18] = END_2;
    s_frame_buff[19] = END_3;
    s_frame_buff[20] = _checksum(s_frame_buff, 20);

    DrvUartPutchars(s_frame_buff, 21);
}

/* Fill triangle */
// TODO: should be int16
void LibEpdFillTriangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x15;

    s_frame_buff[3] = CMD_FILL_TRIANGLE;

    s_frame_buff[4] = (x0 >> 8) & 0xFF;
    s_frame_buff[5] = x0 & 0xFF;
    s_frame_buff[6] = (y0 >> 8) & 0xFF;
    s_frame_buff[7] = y0 & 0xFF;
    s_frame_buff[8] = (x1 >> 8) & 0xFF;
    s_frame_buff[9] = x1 & 0xFF;
    s_frame_buff[10] = (y1 >> 8) & 0xFF;
    s_frame_buff[11] = y1 & 0xFF;
    s_frame_buff[12] = (x2 >> 8) & 0xFF;
    s_frame_buff[13] = x2 & 0xFF;
    s_frame_buff[14] = (y2 >> 8) & 0xFF;
    s_frame_buff[15] = y2 & 0xFF;

    s_frame_buff[16] = END_0;
    s_frame_buff[17] = END_1;
    s_frame_buff[18] = END_2;
    s_frame_buff[19] = END_3;
    s_frame_buff[20] = _checksum(s_frame_buff, 20);

    DrvUartPutchars(s_frame_buff, 21);
}

/* Clear screen using the background colour */
void LibEpdClear(void)
{
    s_frame_buff[0] = START;

    s_frame_buff[1] = 0x00;
    s_frame_buff[2] = 0x09;

    s_frame_buff[3] = CMD_CLEAR;

    s_frame_buff[4] = END_0;
    s_frame_buff[5] = END_1;
    s_frame_buff[6] = END_2;
    s_frame_buff[7] = END_3;
    s_frame_buff[8] = _checksum(s_frame_buff, 8);

    DrvUartPutchars(s_frame_buff, 9);
}

/* Display a single char */
void LibEpdDispChar(unsigned char ch, int x0, int y0)
{
    unsigned char buff[2];

    buff[0] = ch;
    buff[1] = 0;

    LibEpdDispString(buff, x0, y0);
}

/* Display text */
// TODO: Fix the buff size bug
// TODO: should be int16
void LibEpdDispString(const void * p, int x0, int y0)
{
    int string_size;
    unsigned char * ptr = (unsigned char *) p;

    string_size = strlen((const char *) ptr);
    string_size += 14;

    s_frame_buff[0] = START;

    s_frame_buff[1] = (string_size >> 8) & 0xFF;
    s_frame_buff[2] = string_size & 0xFF;

    s_frame_buff[3] = CMD_DRAW_STRING;

    s_frame_buff[4] = (x0 >> 8) & 0xFF;
    s_frame_buff[5] = x0 & 0xFF;
    s_frame_buff[6] = (y0 >> 8) & 0xFF;
    s_frame_buff[7] = y0 & 0xFF;

    strcpy((char *) (&s_frame_buff[8]), (const char *) ptr);

    string_size -= 5;

    s_frame_buff[string_size] = END_0;
    s_frame_buff[string_size + 1] = END_1;
    s_frame_buff[string_size + 2] = END_2;
    s_frame_buff[string_size + 3] = END_3;
    s_frame_buff[string_size + 4] = _checksum(s_frame_buff, string_size + 4);

    DrvUartPutchars(s_frame_buff, string_size + 5);
}

/* Display BMP. Bitmap file name string maximum length is 11 */
// TODO: should be int16
void LibEpdDispBitmap(const void * p, int x0, int y0)
{
    int string_size;
    unsigned char * ptr = (unsigned char *) p;

    string_size = strlen((const char *) ptr);
    string_size += 14;

    s_frame_buff[0] = START;

    s_frame_buff[1] = (string_size >> 8) & 0xFF;
    s_frame_buff[2] = string_size & 0xFF;

    s_frame_buff[3] = CMD_DRAW_BITMAP;

    s_frame_buff[4] = (x0 >> 8) & 0xFF;
    s_frame_buff[5] = x0 & 0xFF;
    s_frame_buff[6] = (y0 >> 8) & 0xFF;
    s_frame_buff[7] = y0 & 0xFF;

    strcpy((char *) (&s_frame_buff[8]), (const char *) ptr);

    string_size -= 5;

    s_frame_buff[string_size] = END_0;
    s_frame_buff[string_size + 1] = END_1;
    s_frame_buff[string_size + 2] = END_2;
    s_frame_buff[string_size + 3] = END_3;
    s_frame_buff[string_size + 4] = _checksum(s_frame_buff, string_size + 4);

    DrvUartPutchars(s_frame_buff, string_size + 5);
}

