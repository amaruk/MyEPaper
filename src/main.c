/***************************************************************************************************
 *
 * @file    main.c
 * @brief   The main entry of the program.
 *
 * @author  amaruk@163.com
 * @date    2017/02/26
 *
 **************************************************************************************************/
#include "common.h"
#include "lib_epd.h"

static void _base_draw(void)
{
    int i, j;

    /* draw pixel */
    lib_epd_clear();
    for (j = 0; j < 600; j += 50)
    {
        for (i = 0; i < 800; i += 50)
        {
            lib_epd_draw_pixel(i, j);
            lib_epd_draw_pixel(i, j + 1);
            lib_epd_draw_pixel(i + 1, j);
            lib_epd_draw_pixel(i + 1, j + 1);
        }
    }
    lib_epd_udpate();

    usleep(3000000);

    /* draw line */
    lib_epd_clear();
    for (i = 0; i < 800; i += 100)
    {
        lib_epd_draw_line(0, 0, i, 599);
        lib_epd_draw_line(799, 0, i, 599);
    }
    lib_epd_udpate();
    usleep(3000000);

    /* fill rect */
    lib_epd_clear();
    lib_epd_set_color(BLACK, WHITE);
    lib_epd_fill_rect(10, 10, 100, 100);

    lib_epd_set_color(DARK_GRAY, WHITE);
    lib_epd_fill_rect(110, 10, 200, 100);

    lib_epd_set_color(GRAY, WHITE);
    lib_epd_fill_rect(210, 10, 300, 100);

    lib_epd_udpate();
    usleep(3000000);

    /* draw circle */
    lib_epd_set_color(BLACK, WHITE);
    lib_epd_clear();
    for (i = 0; i < 300; i += 40)
    {
        lib_epd_draw_circle(399, 299, i);
    }
    lib_epd_udpate();
    usleep(3000000);

    /* fill circle */
    lib_epd_clear();
    for (j = 0; j < 6; j++)
    {
        for (i = 0; i < 8; i++)
        {
            lib_epd_fill_circle(50 + i * 100, 50 + j * 100, 50);
        }
    }
    lib_epd_udpate();
    usleep(3000000);

    /* draw triangle */
    lib_epd_clear();
    for (i = 1; i < 5; i++)
    {
        lib_epd_draw_triangle(399, 249 - i * 50, 349 - i * 50, 349 + i * 50,
                449 + i * 50, 349 + i * 50);
    }
    lib_epd_udpate();
    usleep(3000000);
}
void draw_text_demo(void)
{
    lib_epd_clear();
    printf("Set colours...\n");
    lib_epd_set_color(BLACK, WHITE);
    printf("Display Chinese...\n");
    lib_epd_set_ch_font(GBK32);
    lib_epd_disp_string("中文：狐狸", 0, 50);
    lib_epd_set_ch_font(GBK48);
    lib_epd_disp_string("中文：熊妈", 0, 100);
    lib_epd_set_ch_font(GBK64);
    lib_epd_disp_string("中文：荟雅", 0, 160);

    printf("Display English...\n");
    lib_epd_set_en_font(ASCII32);
    lib_epd_disp_string("ASCII32: Fox!", 0, 300);
    lib_epd_set_en_font(ASCII48);
    lib_epd_disp_string("ASCII48: Carrie!", 0, 350);
    lib_epd_set_en_font(ASCII64);
    lib_epd_disp_string("ASCII64: Aya!", 0, 450);

    usleep(3000000);
    lib_epd_udpate();
}
void draw_bitmap_demo(void)
{
    lib_epd_clear();
    lib_epd_disp_bitmap("PIC4.BMP", 0, 0);
    lib_epd_udpate();
    usleep(5000000);

    lib_epd_clear();
    lib_epd_disp_bitmap("PIC2.BMP", 0, 100);
    lib_epd_disp_bitmap("PIC3.BMP", 400, 100);
    lib_epd_udpate();
    usleep(5000000);

    lib_epd_clear();
    lib_epd_disp_bitmap("FOXB.BMP", 0, 0);
    lib_epd_udpate();
}

void epaperText(char *str, int x, int y)
{
    lib_epd_clear();
    lib_epd_set_color(BLACK, WHITE);

    lib_epd_set_en_font(ASCII32);
    lib_epd_disp_string(str, x, y);

    usleep(1000000);
    lib_epd_udpate();
}

void epaperTest(void)
{
    lib_epd_init();
    lib_epd_wakeup();

    printf("Handshaking...\n");
    lib_epd_handshake();
    usleep(1000000);
    printf("Updating...\n");
    lib_epd_udpate();
    lib_epd_set_memory(MEM_TF);

#if 1
    /* base Draw demo */
    _base_draw();
    /* Draw text demo */
    draw_text_demo();
    /* Draw bitmap */
    draw_bitmap_demo();

    lib_epd_clear();

    lib_epd_close();
#endif

    epaperText("Hello, BBB.", 0, 300);

}

static void _typeCheck(void)
{
    printf("Basic type testing...\n");
    printf("bool:    ");
    (1 == sizeof(bool)) ? printf("PASS\n") : printf("FAIL\n");
    printf("u_int8:  ");
    (1 == sizeof(u_int8)) ? printf("PASS\n") : printf("FAIL\n");
    printf("int8:    ");
    (1 == sizeof(int8)) ? printf("PASS\n") : printf("FAIL\n");
    printf("u_int16: ");
    (2 == sizeof(u_int16)) ? printf("PASS\n") : printf("FAIL\n");
    printf("int16:   ");
    (2 == sizeof(int16)) ? printf("PASS\n") : printf("FAIL\n");
    printf("u_int32: ");
    (4 == sizeof(u_int32)) ? printf("PASS\n") : printf("FAIL\n");
    printf("int32:   ");
    (4 == sizeof(int32)) ? printf("PASS\n") : printf("FAIL\n");
    printf("u_int64: ");
    (8 == sizeof(u_int64)) ? printf("PASS\n") : printf("FAIL\n");
    printf("int64:   ");
    (8 == sizeof(int64)) ? printf("PASS\n") : printf("FAIL\n");
}

int main(void)
{
#ifdef DEBUG
    _typeCheck();
#endif
    
    epaperTest();

    exit(0);
}
