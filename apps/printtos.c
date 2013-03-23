/**
 * printtos.c
 *
 * Date: March 11, 2013
 * Author: Jedd Haberstro
 */

#include <thread.h>
#include <stdio.h>

static void myfuncA() {
    //unsigned int* stkptr = 0;
    //asm("mov %[result], sp" : [result] "=r" (stkptr) :);
    register unsigned int* stkptr asm ("sp");
    printf("SP: %d\n", stkptr);
    printf(
        "1. %d = %d\n2. %d = %d\n3. %d = %d\n4. %d = %d\n5. %d = %d\n",
        stkptr, *(stkptr),
        stkptr + 1, *(stkptr + 1),
        stkptr + 2, *(stkptr + 2),
        stkptr + 3, *(stkptr + 3),
        stkptr + 4, *(stkptr + 4)
    );
}

static void myprogA() {
    register unsigned int* stkptr asm ("sp");
    printf("SP: %d\n", stkptr);
    printf(
        "1. %d = %d\n2. %d = %d\n3. %d = %d\n4. %d = %d\n5. %d = %d\n\n",
        stkptr, *(stkptr),
        stkptr + 1, *(stkptr + 1),
        stkptr + 2, *(stkptr + 2),
        stkptr + 3, *(stkptr + 3),
        stkptr + 4, *(stkptr + 4)
    );
    myfuncA();
}

void printtos() {
    tid_typ myProgAID = create(myprogA, 1024, 20, "myprogA()", 0);
    syscall r = resume(myProgAID);
}

