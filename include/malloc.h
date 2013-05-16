#ifndef _MALLOC_H
#define _MALLOC_H

#define BLOCKLOG  12
#define BLOCKSIZE 4096
#define HEAP      4194304
#define BLOCKIFY(SIZE) (((SIZE) + BLOCKSIZE - 1) / BLOCKSIZE)

#define FINAL_FREE_BLOCKS 8

#define BLOCK(A)  (((char *) (A) - _heapbase) / BLOCKSIZE + 1)
#define ADDRESS(B) ((void *) (((B) - 1) * BLOCKSIZE + _heapbase))

extern void *(*_morecore)(long);
extern void *_default_morecore(long);

union info {
    struct {
        int type;

        union {
            struct {
                int nfree;
                int first;
            } frag;
            int size;
        } info;
    } busy;

    struct {
        int size;
        int next;
        int prev;
    } free;
};

/* Doubly linked list of free fragments. */
struct list {
    struct list *next;
    struct list *prev;
};

/* List of blocks allocated with memalign() */
struct alignlist {
    struct alignlist *next;
    int aligned;
    int exact;
};

extern struct alignlist *_aligned_blocks;

extern char *_heapbase;
extern union info *_heapinfo;
extern int _heapindex;
extern int _heaplimit;
extern int _fragblocks[];
extern struct list _fraghead[];

#endif
