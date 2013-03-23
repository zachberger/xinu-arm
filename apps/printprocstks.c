/*
 * printprocstks.c
 *
 * Date: March 13, 2013
 * Author: Jedd Haberstro
 */

#include <stdio.h>
#include <thread.h>

static void myprogA() {
    tid_typ tid = gettid();
    struct thrent* thr = &thrtab[tid];
    //unsigned int* stkptr = 0;
    //unsigned int stklim = 0;
    //asm("mov %[result], sp" : [result] "=r" (stkptr) :);
    //asm("mov %[result], sl" : [result] "=r" (stklim) :);
    register unsigned int* stkptr asm ("sp");
    printf("PID: %d\n", tid);
    printf("\tStack:\tBase = %u\n", (unsigned int)thr->stkbase);
    printf("\t\tLength = %u\n", thr->stklen);
    printf("\t\tLimit = %u\n", (unsigned int)thr->stkbase - MINSTK);
    printf("\t\tPtr = %u\n", stkptr);
}

void printprocstks() {
    tid_typ t1 = create(myprogA, 1000000, 20, "myprogA - 1", 0);
    tid_typ t2 = create(myprogA, 1000000, 20, "myprogA - 2", 0);
    tid_typ t3 = create(myprogA, 1000000, 20, "myprogA - 3", 0);
    syscall r1 = resume(t1);
    syscall r2 = resume(t2);
    syscall r3 = resume(t3);
}
