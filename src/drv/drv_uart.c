/***************************************************************************************************
 *
 * @file    drv_uart.c
 * @brief   The driver of UART.
 *          The following articles were taken as reference to make this driver:
 *          http://blog.csdn.net/wangzhen209/article/details/32715467
 *          http://blog.csdn.net/zhanghuiliang/article/details/2467040
 *          http://blog.sina.com.cn/s/blog_4673bfa50100b5y4.html
 *          http://www.cnblogs.com/chengmin/p/3818133.html
 *          http://blog.csdn.net/w282529350/article/details/7386616
 *
 * @author  amaruk@163.com
 * @date    2017/02/26
 *
 **************************************************************************************************/

#include "common.h"

static int s_uart_fd;

#ifdef POSIX_STD
const int c_speed_arr[] =
{ B38400, B19200, B9600, B4800, B2400, B1200, B300,
  B38400, B19200, B9600, B4800, B2400, B1200, B300,};
const int c_name_arr[] =
{ 38400, 19200, 9600, 4800, 2400, 1200, 300, 38400, 19200, 9600, 4800, 2400,
  1200, 300,};
#else
const int c_speed_arr[] =
{
B0, B50, B75, B110,
B134, B150, B200, B300,
B600, B1200, B1800, B2400,
B4800, B9600, B19200, B38400,/*the POSIX std*/
B57600, B115200, B3000000 //,	B6000000, 	B12000000/*pl2303 ext*/
};
const int c_name_arr[] =
{ 0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200,
  38400, /*the POSIX std*/
  57600, 115200, 3000000 //,	6000000,	12000000 /*pl2303 ext*/
};
#endif

void DrvUartSetSpeed(int fd, int speed)
{
    int i;
    int status;
    struct termios Opt;
    tcgetattr(fd, &Opt);
    for (i = 0; i < sizeof(c_speed_arr) / sizeof(int); i++)
    {
        if (speed == c_name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);   //Update the options and do it NOW
            cfsetispeed(&Opt, c_speed_arr[i]);
            cfsetospeed(&Opt, c_speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if (status != 0)
                perror("tcsetattr fd1");
            return;
        }
        tcflush(fd, TCIOFLUSH);
    }
}

int DrvUartSetParity(int fd, int databits, int stopbits, int parity)
{
    struct termios options;
    if (tcgetattr(fd, &options) != 0)
    {
        perror("SetupSerial 1");
        return (FALSE);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr, "Unsupported data size\n");
        return (FALSE);
    }
    switch (parity)
    {
    case 'n':
    case 'N':
        options.c_cflag &= ~PARENB; /* Clear parity enable */
        options.c_iflag &= ~INPCK; /* Enable parity checking */
        break;
    case 'o':
    case 'O':
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK; /* Disnable parity checking */
        break;
    case 'e':
    case 'E':
        options.c_cflag |= PARENB; /* Enable parity */
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK; /* Disnable parity checking */
        break;
    case 'S':
    case 's': /*as no parity*/
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported parity\n");
        return (FALSE);
    }

    switch (stopbits)
    {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported stop bits\n");
        return (FALSE);
    }
    /* Set input parity option */
    if (parity != 'n')
        options.c_iflag |= INPCK;
    options.c_cc[VTIME] = 150; // 15 seconds
    options.c_cc[VMIN] = 0;

    tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("SetupSerial 3");
        return (FALSE);
    }
    return (TRUE);
}

int DrvUartSetOthers(int fd)
{
    struct termios options;
    if (tcgetattr(fd, &options) != 0)
    {
        perror("SetupSerial 1");
        return (FALSE);
    }

#ifndef PLATFORM_CYGWIN
    /* Disable auto echo */
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOCTL
            | ECHOPRT | ECHOKE | ISIG);
#endif
    /* Disable auto transfer of "0x0d" and "0x0a" */
    options.c_oflag &= ~(ONLCR | OCRNL);

    tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("SetupSerial 3");
        return (FALSE);
    }
    return (TRUE);
}

int DrvUartOpenDev(char *Dev)
{
    int fd = open(Dev, O_RDWR);         //| O_NOCTTY | O_NDELAY
    if (-1 == fd)
    {
        perror("Can't Open Serial Port");
        return -1;
    } else
        return fd;

}

int DrvUartKill(void)
{
    close(s_uart_fd);
    return TRUE;
}

/* Transmit bytes */
int DrvUartPutchars(const unsigned char * ptr, int n)
{
    return write(s_uart_fd, ptr, n);
}

/* Receive bytes */
int DrvUartGetChars(unsigned char * ptr)
{
    int nread = 0;
    if ((nread = read(s_uart_fd, ptr, 512)) > 0)
    {
        //printf("\nLen %d ", nread);
        *(ptr + nread) = '\0';
        printf("DrvUartGetChars: [%s]\n", ptr);
    } else
    {
        printf("DrvUartGetChars error\n");
    }
    return nread;
}

int DrvUartInit(char *dev_name, int speed, int databits, int stopbits,
        int parity)
{

    printf("Opening %s\n", dev_name);
    s_uart_fd = DrvUartOpenDev(dev_name);

    if (s_uart_fd > 0)
    {
        DrvUartSetSpeed(s_uart_fd, speed);
    } else
    {
        printf("ERROR: Can't Open Serial Port!\n");
        return FALSE;
    }

    if (DrvUartSetParity(s_uart_fd, databits, stopbits, parity) == FALSE)
    {
        printf("ERROR: Set Parity Error\n");
        return FALSE;
    }

    DrvUartSetOthers(s_uart_fd);

    return TRUE;
}
