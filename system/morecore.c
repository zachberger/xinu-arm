#include <malloc.h>

extern void *sbrk(int);

void *_default_morecore(long size) {
    void *result;
    result = sbrk(size);
    if (result == (void *) -1) return 0;
    return result;
}
