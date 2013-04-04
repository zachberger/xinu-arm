/**
 * @file     xsh_ledoff.c
 * @provides xsh_ledoff.
 *
 * $Id: xsh_led.c 2157 2010-01-19 00:40:07Z brylow $
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include "../system/platforms/raspberry-pi/gpio.h"
#include <stdio.h>
#include <string.h>

/**
 * Shell command (led DESCRIPTOR STATE) turns a specified led on or off.
 * @param nargs number of arguments in args array
 * @param args  array of arguments
 * @return non-zero value on error
 */
shellcmd xsh_ledoff(int nargs, char *args[])
{
    GPIOSET(16);
}
