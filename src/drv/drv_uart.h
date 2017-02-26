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

int drv_uart_init(char *dev_name, int speed, int databits, int stopbits,
        int parity);
int drv_uart_kill(void);
int drv_uart_putchars(const unsigned char * ptr, int n);
int drv_uart_getchars(const unsigned char * ptr);

#endif /* DRV_UART_H_ */
