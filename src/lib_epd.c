#include "common.h"
#include "lib_epd.h"
#include "drv_uart.h"

static int wake_up = 0; /* Wake up pin */
static int reset = 0; /* Reset pin */

/*
 command define
 */
static const unsigned char _cmd_handshake[8] =
{ 0xA5, 0x00, 0x09, CMD_HANDSHAKE, 0xCC, 0x33, 0xC3, 0x3C };     //CMD_HANDSHAKE
static const unsigned char _cmd_read_baud[8] =
{ 0xA5, 0x00, 0x09, CMD_READ_BAUD, 0xCC, 0x33, 0xC3, 0x3C };     //CMD_READ_BAUD
static const unsigned char _cmd_stopmode[8] =
{ 0xA5, 0x00, 0x09, CMD_STOPMODE, 0xCC, 0x33, 0xC3, 0x3C };       //CMD_STOPMODE
static const unsigned char _cmd_update[8] =
{ 0xA5, 0x00, 0x09, CMD_UPDATE, 0xCC, 0x33, 0xC3, 0x3C };           //CMD_UPDATE
static const unsigned char _cmd_load_font[8] =
{ 0xA5, 0x00, 0x09, CMD_LOAD_FONT, 0xCC, 0x33, 0xC3, 0x3C };     //CMD_LOAD_FONT
static const unsigned char _cmd_load_pic[8] =
{ 0xA5, 0x00, 0x09, CMD_LOAD_PIC, 0xCC, 0x33, 0xC3, 0x3C };       //CMD_LOAD_PIC

/* Command data */
static unsigned char _cmd_buff[CMD_SIZE];

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
	FILE  *fd=NULL;

	fd=fopen(SYSFS_UART_DEV,"w");
	fwrite("BB-UART4",sizeof(int),8,fd);   /*BB-UART4ÓÐ8¸ö×Ö·û*/
	fclose(fd);

	drv_uart_init("/dev/ttyO4", 115200, 8, 1, 'N');
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
	memcpy(_cmd_buff, _cmd_handshake, 8);
	_cmd_buff[8] = _checksum(_cmd_buff, 8);

	drv_uart_putchars(_cmd_buff, 9);
	drv_uart_getchars(_cmd_buff);
}

/* Set baudrate */
void lib_epd_set_baud(long baud)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0D;

	_cmd_buff[3] = CMD_SET_BAUD;

	_cmd_buff[4] = (baud >> 24) & 0xFF;
	_cmd_buff[5] = (baud >> 16) & 0xFF;
	_cmd_buff[6] = (baud >> 8) & 0xFF;
	_cmd_buff[7] = baud & 0xFF;

	_cmd_buff[8] = FRAME_E0;
	_cmd_buff[9] = FRAME_E1;
	_cmd_buff[10] = FRAME_E2;
	_cmd_buff[11] = FRAME_E3;
	_cmd_buff[12] = _checksum(_cmd_buff, 12);

	drv_uart_putchars(_cmd_buff, 13);

	usleep(10000);
	//FOX uart.baud(baud);
}

/* Read baudrate */
void lib_epd_read_baud(void)
{
	memcpy(_cmd_buff, _cmd_read_baud, 8);
	_cmd_buff[8] = _checksum(_cmd_buff, 8);

	drv_uart_putchars(_cmd_buff, 9);
}

/* Choose memory to be used.
 * mode: MEM_TF or MEM_NAND */
void lib_epd_set_memory(unsigned char mode)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;

	_cmd_buff[3] = CMD_MEMORYMODE;

	_cmd_buff[4] = mode;

	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;
	_cmd_buff[9] = _checksum(_cmd_buff, 9);

	drv_uart_putchars(_cmd_buff, 10);
}

/* Enter stop mode */
void lib_epd_enter_stopmode(void)
{
	memcpy(_cmd_buff, _cmd_stopmode, 8);
	_cmd_buff[8] = _checksum(_cmd_buff, 8);

	drv_uart_putchars(_cmd_buff, 9);
}

/* Update the e-paper's screen:
 * Flush buffer to screen.
 */
void lib_epd_udpate(void)
{
	memcpy(_cmd_buff, _cmd_update, 8);
	_cmd_buff[8] = _checksum(_cmd_buff, 8);

	drv_uart_putchars(_cmd_buff, 9);
}

/* Normal screen or upside down screen */
void lib_epd_screen_rotation(unsigned char mode)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;

	_cmd_buff[3] = CMD_SCREEN_ROTATION;

	_cmd_buff[4] = mode;

	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;
	_cmd_buff[9] = _checksum(_cmd_buff, 9);

	drv_uart_putchars(_cmd_buff, 10);
}

/* Load font from TF to NAND */
void lib_epd_load_font(void)
{
	memcpy(_cmd_buff, _cmd_load_font, 8);
	_cmd_buff[8] = _checksum(_cmd_buff, 8);

	drv_uart_putchars(_cmd_buff, 9);
}

/* Load BMP from TF to NAND */
void lib_epd_load_pic(void)
{
	memcpy(_cmd_buff, _cmd_load_pic, 8);
	_cmd_buff[8] = _checksum(_cmd_buff, 8);

	drv_uart_putchars(_cmd_buff, 9);
}

/* Set fore-ground and back-ground colours */
void lib_epd_set_color(unsigned char color, unsigned char bkcolor)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0B;

	_cmd_buff[3] = CMD_SET_COLOR;

	_cmd_buff[4] = color;
	_cmd_buff[5] = bkcolor;

	_cmd_buff[6] = FRAME_E0;
	_cmd_buff[7] = FRAME_E1;
	_cmd_buff[8] = FRAME_E2;
	_cmd_buff[9] = FRAME_E3;
	_cmd_buff[10] = _checksum(_cmd_buff, 10);

	drv_uart_putchars(_cmd_buff, 11);
}

/* Set English font */
void lib_epd_set_en_font(unsigned char font)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;

	_cmd_buff[3] = CMD_SET_EN_FONT;

	_cmd_buff[4] = font;

	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;
	_cmd_buff[9] = _checksum(_cmd_buff, 9);

	drv_uart_putchars(_cmd_buff, 10);
}

/* Set Chinese font */
void lib_epd_set_ch_font(unsigned char font)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;

	_cmd_buff[3] = CMD_SET_CH_FONT;

	_cmd_buff[4] = font;

	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;
	_cmd_buff[9] = _checksum(_cmd_buff, 9);

	drv_uart_putchars(_cmd_buff, 10);
}

/* Draw single pixel */
void lib_epd_draw_pixel(int x0, int y0)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0D;

	_cmd_buff[3] = CMD_DRAW_PIXEL;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;

	_cmd_buff[8] = FRAME_E0;
	_cmd_buff[9] = FRAME_E1;
	_cmd_buff[10] = FRAME_E2;
	_cmd_buff[11] = FRAME_E3;
	_cmd_buff[12] = _checksum(_cmd_buff, 12);

	drv_uart_putchars(_cmd_buff, 13);
}

/* Draw line */
void lib_epd_draw_line(int x0, int y0, int x1, int y1)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x11;

	_cmd_buff[3] = CMD_DRAW_LINE;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;

	_cmd_buff[12] = FRAME_E0;
	_cmd_buff[13] = FRAME_E1;
	_cmd_buff[14] = FRAME_E2;
	_cmd_buff[15] = FRAME_E3;
	_cmd_buff[16] = _checksum(_cmd_buff, 16);

	drv_uart_putchars(_cmd_buff, 17);
}

/* Fill rectangle */
void lib_epd_fill_rect(int x0, int y0, int x1, int y1)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x11;

	_cmd_buff[3] = CMD_FILL_RECT;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;

	_cmd_buff[12] = FRAME_E0;
	_cmd_buff[13] = FRAME_E1;
	_cmd_buff[14] = FRAME_E2;
	_cmd_buff[15] = FRAME_E3;
	_cmd_buff[16] = _checksum(_cmd_buff, 16);

	drv_uart_putchars(_cmd_buff, 17);
}

/* Draw circle */
void lib_epd_draw_circle(int x0, int y0, int r)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0F;

	_cmd_buff[3] = CMD_DRAW_CIRCLE;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (r >> 8) & 0xFF;
	_cmd_buff[9] = r & 0xFF;

	_cmd_buff[10] = FRAME_E0;
	_cmd_buff[11] = FRAME_E1;
	_cmd_buff[12] = FRAME_E2;
	_cmd_buff[13] = FRAME_E3;
	_cmd_buff[14] = _checksum(_cmd_buff, 14);

	drv_uart_putchars(_cmd_buff, 15);
}

/* Fill circle */
void lib_epd_fill_circle(int x0, int y0, int r)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0F;

	_cmd_buff[3] = CMD_FILL_CIRCLE;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (r >> 8) & 0xFF;
	_cmd_buff[9] = r & 0xFF;

	_cmd_buff[10] = FRAME_E0;
	_cmd_buff[11] = FRAME_E1;
	_cmd_buff[12] = FRAME_E2;
	_cmd_buff[13] = FRAME_E3;
	_cmd_buff[14] = _checksum(_cmd_buff, 14);

	drv_uart_putchars(_cmd_buff, 15);
}

/* Draw triangle */
void lib_epd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x15;

	_cmd_buff[3] = CMD_DRAW_TRIANGLE;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;
	_cmd_buff[12] = (x2 >> 8) & 0xFF;
	_cmd_buff[13] = x2 & 0xFF;
	_cmd_buff[14] = (y2 >> 8) & 0xFF;
	_cmd_buff[15] = y2 & 0xFF;

	_cmd_buff[16] = FRAME_E0;
	_cmd_buff[17] = FRAME_E1;
	_cmd_buff[18] = FRAME_E2;
	_cmd_buff[19] = FRAME_E3;
	_cmd_buff[20] = _checksum(_cmd_buff, 20);

	drv_uart_putchars(_cmd_buff, 21);
}

/* Fill triangle */
void lib_epd_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x15;

	_cmd_buff[3] = CMD_FILL_TRIANGLE;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;
	_cmd_buff[12] = (x2 >> 8) & 0xFF;
	_cmd_buff[13] = x2 & 0xFF;
	_cmd_buff[14] = (y2 >> 8) & 0xFF;
	_cmd_buff[15] = y2 & 0xFF;

	_cmd_buff[16] = FRAME_E0;
	_cmd_buff[17] = FRAME_E1;
	_cmd_buff[18] = FRAME_E2;
	_cmd_buff[19] = FRAME_E3;
	_cmd_buff[20] = _checksum(_cmd_buff, 20);

	drv_uart_putchars(_cmd_buff, 21);
}

/* Clear screen using the background colour */
void lib_epd_clear(void)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x09;

	_cmd_buff[3] = CMD_CLEAR;

	_cmd_buff[4] = FRAME_E0;
	_cmd_buff[5] = FRAME_E1;
	_cmd_buff[6] = FRAME_E2;
	_cmd_buff[7] = FRAME_E3;
	_cmd_buff[8] = _checksum(_cmd_buff, 8);

	drv_uart_putchars(_cmd_buff, 9);
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
void lib_epd_disp_string(const void * p, int x0, int y0)
{
	int string_size;
	unsigned char * ptr = (unsigned char *) p;

	string_size = strlen((const char *) ptr);
	string_size += 14;

	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = (string_size >> 8) & 0xFF;
	_cmd_buff[2] = string_size & 0xFF;

	_cmd_buff[3] = CMD_DRAW_STRING;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;

	strcpy((char *) (&_cmd_buff[8]), (const char *) ptr);

	string_size -= 5;

	_cmd_buff[string_size] = FRAME_E0;
	_cmd_buff[string_size + 1] = FRAME_E1;
	_cmd_buff[string_size + 2] = FRAME_E2;
	_cmd_buff[string_size + 3] = FRAME_E3;
	_cmd_buff[string_size + 4] = _checksum(_cmd_buff, string_size + 4);

	drv_uart_putchars(_cmd_buff, string_size + 5);
}

/* Display BMP */
void lib_epd_disp_bitmap(const void * p, int x0, int y0)
{
	int string_size;
	unsigned char * ptr = (unsigned char *) p;

	string_size = strlen((const char *) ptr);
	string_size += 14;

	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = (string_size >> 8) & 0xFF;
	_cmd_buff[2] = string_size & 0xFF;

	_cmd_buff[3] = CMD_DRAW_BITMAP;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;

	strcpy((char *) (&_cmd_buff[8]), (const char *) ptr);

	string_size -= 5;

	_cmd_buff[string_size] = FRAME_E0;
	_cmd_buff[string_size + 1] = FRAME_E1;
	_cmd_buff[string_size + 2] = FRAME_E2;
	_cmd_buff[string_size + 3] = FRAME_E3;
	_cmd_buff[string_size + 4] = _checksum(_cmd_buff, string_size + 4);

	drv_uart_putchars(_cmd_buff, string_size + 5);
}

