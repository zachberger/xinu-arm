#include <stdio.h>
#include <memory.h>

// TODO fix for processes
static void *curr_brk = (void *) &_end;

/*
** Sets the end of the data segment to the value specified
** by addr. On success, brk() returns zero. On error, -1 is
** returned.
*/
int brk(void *addr) {
    void *new_brk;
    if (curr_brk == addr) return 0;
    new_brk = (void *) addr;
    curr_brk = new_brk;
    if (new_brk < addr) return -1;
    return 0;
}

/*
** Increments the program's data space by incr bytes. On
** success, sbrk() returns the previous program break.
** On error, (void *) -1 is returned.
*/
void *sbrk(intptr_t incr) {
    void *new_brk = (void *) (curr_brk + incr);

    if ((incr > 0 && (new_brk < curr_brk)) ||
        (incr < 0 && (new_brk > curr_brk))) {
        kprintf("sbrk: [failed] incr\n");
        return ((void *) -1);
    }
    if (!brk(new_brk)) {
        kprintf("sbrk: [success] %x\n", curr_brk);
        return curr_brk;
    } else {
        kprintf("sbrk: [failed] default\n");
        return ((void *) -1);
    }
}
