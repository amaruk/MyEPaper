/***************************************************************************************************
 *
 * @file    drv_uart.c
 * @brief   API of the UART driver.
 *
 * @author  amaruk@163.com
 * @date    2017/02/26
 *
 **************************************************************************************************/

#ifndef DRV_UART_H_
#define DRV_UART_H_

int DrvUartInit(char *dev_name, int speed, int databits, int stopbits,
        int parity);
int DrvUartKill(void);
int DrvUartPutchars(const unsigned char * ptr, int n);
int DrvUartGetChars(const unsigned char * ptr);

#endif /* DRV_UART_H_ */
