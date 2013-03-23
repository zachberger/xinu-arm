/**
 * xsh_printprocstks.c
 *
 * Date: March 13, 2013
 * Author: Jedd Haberstro
 */

#include <shell.h>

extern void printprocstks();

shellcmd xsh_printprocstks(int nargs, char** args) {
    printprocstks();
    return OK;
}
