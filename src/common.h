/*
 * common.h
 *
 *  Created on: Apr 8, 2016
 *      Author: fox
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#define PIN_LOW		0
#define PIN_HIGH	1

#define TRUE 1
#define FALSE 0

#if defined(PLATFORM_UBUNTU)
typedef unsigned char   bool;
typedef unsigned char   u_int8;
typedef signed char     int8;
typedef unsigned short  u_int16;
typedef signed short    int16;
typedef unsigned int    u_int32;
typedef signed int      int32;
typedef unsigned long   u_int64;
typedef signed long     int64;
#elif defined(PLATFORM_CYGWIN)
typedef unsigned char   bool;
typedef unsigned char   u_int8;
typedef signed char     int8;
typedef unsigned short  u_int16;
typedef signed short    int16;
typedef unsigned int    u_int32;
typedef signed int      int32;
typedef unsigned long   u_int64;
typedef signed long     int64;
#endif

#endif /* COMMON_H_ */
