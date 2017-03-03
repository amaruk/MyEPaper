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

static void _BaseDraw(void)
{
    int i, j;

    /* draw pixel */
    LibEpdClear();
    for (j = 0; j < 600; j += 50)
    {
        for (i = 0; i < 800; i += 50)
        {
            LibEpdDrawPixel(i, j);
            LibEpdDrawPixel(i, j + 1);
            LibEpdDrawPixel(i + 1, j);
            LibEpdDrawPixel(i + 1, j + 1);
        }
    }
    LibEpdUpdate();

    usleep(3000000);

    /* draw line */
    LibEpdClear();
    for (i = 0; i < 800; i += 100)
    {
        LibEpdDrawLine(0, 0, i, 599);
        LibEpdDrawLine(799, 0, i, 599);
    }
    LibEpdUpdate();
    usleep(3000000);

    /* fill rect */
    LibEpdClear();
    LibEpdSetColor(BLACK, WHITE);
    LibEpdFillRect(10, 10, 100, 100);

    LibEpdSetColor(DARK_GRAY, WHITE);
    LibEpdFillRect(110, 10, 200, 100);

    LibEpdSetColor(GRAY, WHITE);
    LibEpdFillRect(210, 10, 300, 100);

    LibEpdUpdate();
    usleep(3000000);

    /* draw circle */
    LibEpdSetColor(BLACK, WHITE);
    LibEpdClear();
    for (i = 0; i < 300; i += 40)
    {
        LibEpdDrawCircle(399, 299, i);
    }
    LibEpdUpdate();
    usleep(3000000);

    /* fill circle */
    LibEpdClear();
    for (j = 0; j < 6; j++)
    {
        for (i = 0; i < 8; i++)
        {
            LibEpdFillCircle(50 + i * 100, 50 + j * 100, 50);
        }
    }
    LibEpdUpdate();
    usleep(3000000);

    /* draw triangle */
    LibEpdClear();
    for (i = 1; i < 5; i++)
    {
        LibEpdDrawTriangle(399, 249 - i * 50, 349 - i * 50, 349 + i * 50,
                449 + i * 50, 349 + i * 50);
    }
    LibEpdUpdate();
    usleep(3000000);
}
void DrawTextDemo(void)
{
    LibEpdClear();
    printf("Set colours...\n");
    LibEpdSetColor(BLACK, WHITE);
    printf("Display Chinese...\n");
    LibEpdSetChFont(GBK32);
    LibEpdDispString("中文：狐狸", 0, 50);
    LibEpdSetChFont(GBK48);
    LibEpdDispString("中文：熊妈", 0, 100);
    LibEpdSetChFont(GBK64);
    LibEpdDispString("中文：荟雅", 0, 160);

    printf("Display English...\n");
    LibEpdSetEnFont(ASCII32);
    LibEpdDispString("ASCII32: Fox!", 0, 300);
    LibEpdSetEnFont(ASCII48);
    LibEpdDispString("ASCII48: Carrie!", 0, 350);
    LibEpdSetEnFont(ASCII64);
    LibEpdDispString("ASCII64: Aya!", 0, 450);

    usleep(3000000);
    LibEpdUpdate();
}

void DrawBitmapDemo(void)
{
    LibEpdClear();
    LibEpdDispBitmap("PIC4.BMP", 0, 0);
    LibEpdUpdate();
    usleep(5000000);

    LibEpdClear();
    LibEpdDispBitmap("PIC2.BMP", 0, 100);
    LibEpdDispBitmap("PIC3.BMP", 400, 100);
    LibEpdUpdate();
    usleep(5000000);

    LibEpdClear();
    LibEpdDispBitmap("FOXB.BMP", 0, 0);
    LibEpdUpdate();
}

void EpaperText(char *str, int x, int y)
{
    LibEpdClear();
    LibEpdSetColor(BLACK, WHITE);

    LibEpdSetEnFont(ASCII32);
    LibEpdDispString(str, x, y);

    usleep(1000000);
    LibEpdUpdate();
}

void EpaperTest(void)
{
    LibEpdInit();
    LibEpdWakeup();

    printf("Handshaking...\n");
    LibEpdHandshake();
    usleep(1000000);
    printf("Updating...\n");
    LibEpdUpdate();
    LibEpdSetMemory(MEM_TF);

#if 1
    /* base Draw demo */
    _BaseDraw();
    /* Draw text demo */
    DrawTextDemo();
    /* Draw bitmap */
    DrawBitmapDemo();

    LibEpdClear();

    LibEpdClose();
#endif

    EpaperText("Hello, BBB.", 0, 300);

}

static void _TypeCheck(void)
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
    _TypeCheck();
#endif
    
    EpaperTest();

    exit(0);
}
