/**
 * xsh_fbtest.c
 *
 * Date: March 22, 2013
 * Author: Jedd Haberstro
 */

#include <shell.h>

extern void fbtest();

shellcmd xsh_fbtest(int nargs, char** args) {
    fbtest();
    return OK;
}
