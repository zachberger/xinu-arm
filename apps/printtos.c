/**
 * printtos.c
 *
 * Date: March 11, 2013
 * Author: Jedd Haberstro
 */

#include <thread.h>
#include <stdio.h>

static void myfuncA() {
    int dummy[2];

    int d = &(dummy[0]) - &(dummy[1]);
    unsigned int* sp = (&dummy[0]) + d;
    printf(
        "1. %d\n2. %d\n3. %d\n4. %d\n5. %d\n",
        *(sp), *(sp + (d * 1)), *(sp + (d * 2)), *(sp + (d * 3)), *(sp + (d * 4))
    );
}

static void myprogA() {
    {
        int dummy[2];

        int d = &(dummy[0]) - &(dummy[1]);
        unsigned int* sp = (&dummy[0]) + d;
        printf(
            "1. %d\n2. %d\n3. %d\n4. %d\n5. %d\n",
            *(sp), *(sp + (d * 1)), *(sp + (d * 2)), *(sp + (d * 3)), *(sp + (d * 4))
        );
    }

    myfuncA();
}

void printtos() {
    tid_typ myProgAID = create(myprogA, 1024, 20, "myprogA()", 0);
    syscall r = resume(myProgAID);
}

