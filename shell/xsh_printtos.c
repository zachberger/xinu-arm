/**
 * xsh_printtos.c
 *
 * Date: March 11, 2013
 * Author: Jedd Haberstr
 */

#include <shell.h>

extern void printtos();

shellcmd xsh_printtos(int nargs, char** args) {
    printtos();
    return OK;
}
