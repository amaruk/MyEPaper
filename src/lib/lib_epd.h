/***************************************************************************************************
 *
 * @file    lib_epd.h
 * @brief   The API of waveshare 4.3 inch E-Paper screen library.
 *
 * @author  amaruk@163.com
 * @date    2017/02/26
 *
 **************************************************************************************************/

#ifndef LIB_EPD_H
#define LIB_EPD_H



/* Color define */
#define    WHITE                    0x03
#define    GRAY                     0x02
#define    DARK_GRAY                0x01
#define    BLACK                    0x00

/* Frame buff size */
#define     FRAME_BUFF_SIZE         512	
/* Frame start byte */
#define     START                   0xA5
/* Frame end sequence */
#define     END_0                   0xCC
#define     END_1                   0x33
#define     END_2                   0xC3
#define     END_3                   0x3C
/* Frame command definitions: System configuration */
#define     CMD_HANDSHAKE           0x00    //handshake
#define     CMD_SET_BAUD            0x01    //set baudrate
#define     CMD_READ_BAUD           0x02    //read baud
#define     CMD_GET_MEM_MODE        0x07    // TODO: Read memory mode
#define     CMD_SET_MEM_MODE        0x07    //set memory mode
#define     CMD_STOP_MODE           0x08    //enter stop mode
#define     CMD_UPDATE              0x0A    //update screen
#define     CMD_GET_SCR_ROTATION    0x0C    // TODO: READ screen rotation
#define     CMD_SET_SCR_ROTATION    0x0D    //set screen rotation
#define     CMD_LOAD_FONT           0x0E    //load font
#define     CMD_LOAD_PIC            0x0F    //load picture
/* Frame command definitions: Display configuration */
#define     CMD_SET_COLOR           0x10    //set color
#define     CMD_GET_COLOR           0x11    // TODO: get color
#define     CMD_GET_EN_FONT         0x1C    // TODO: get English font size. 1:32dot 2:48dot 3:64dot
#define     CMD_GET_CH_FONT         0x1D    // TODO: get Chinese font size. 1:32dot 2:48dot 3:64dot
#define     CMD_SET_EN_FONT         0x1E    //set english font
#define     CMD_SET_CH_FONT         0x1F    //set chinese font
/* Frame command definitions: Drawing */
#define     CMD_DRAW_PIXEL          0x20    //set pixel
#define     CMD_DRAW_LINE           0x22    //draw line
#define     CMD_FILL_RECT           0x24    //fill rectangle
#define     CMD_DRAW_RECT           0x25    // TODO: draw rectangle
#define     CMD_DRAW_CIRCLE         0x26    //draw circle
#define     CMD_FILL_CIRCLE         0x27    //fill circle
#define     CMD_DRAW_TRIANGLE       0x28    //draw triangle
#define     CMD_FILL_TRIANGLE       0x29    //fill triangle
#define     CMD_CLEAR               0x2E    //clear screen use background color
/* Frame command definitions: Text */
#define     CMD_DRAW_STRING         0x30    //draw string
/* Frame command definitions: image */
#define     CMD_DRAW_BITMAP         0x70    //draw bitmap
/* Frame command data dummy value */
#define     CMD_DATA_BYTE           0x00


/*
 FONT
 */
#define    GBK32                              0x01
#define    GBK48                              0x02
#define    GBK64                              0x03

#define    ASCII32                            0x01
#define    ASCII48                            0x02
#define    ASCII64                            0x03

/* Memory Mode */
#define    MEM_NAND                           0
#define    MEM_TF                             1

/*
 set screen rotation
 */
#define    EPD_NORMAL                         0              //screen normal
#define    EPD_INVERSION                      1              //screen inversion

void lib_epd_init(void);
void lib_epd_close(void);
void lib_epd_reset(void);
void lib_epd_wakeup(void);

void lib_epd_handshake(void);
void lib_epd_set_baud(long baud);
void lib_epd_read_baud(void);
void lib_epd_set_memory(unsigned char mode);
void lib_epd_enter_stopmode(void);
void lib_epd_udpate(void);
void lib_epd_screen_rotation(unsigned char mode);
void lib_epd_load_font(void);
void lib_epd_load_pic(void);

void lib_epd_set_color(unsigned char color, unsigned char bkcolor);
void lib_epd_set_en_font(unsigned char font);
void lib_epd_set_ch_font(unsigned char font);

void lib_epd_draw_pixel(int x0, int y0);
void lib_epd_draw_line(int x0, int y0, int x1, int y1);
void lib_epd_fill_rect(int x0, int y0, int x1, int y1);
void lib_epd_draw_circle(int x0, int y0, int r);
void lib_epd_fill_circle(int x0, int y0, int r);
void lib_epd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2);
void lib_epd_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2);
void lib_epd_clear(void);

void lib_epd_disp_char(unsigned char ch, int x0, int y0);
void lib_epd_disp_string(const void * p, int x0, int y0);

void lib_epd_disp_bitmap(const void * p, int x0, int y0);

#endif

