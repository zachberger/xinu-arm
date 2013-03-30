/**
 * Xinu Physical Memory Manager/Frame Allocator
 *
 * Author: Jedd Haberstro
 * Date: March 26, 2013
 **/

#include <stdint.h>

#define PMM_FRAME_SIZE 4096

#define PMM_ALIGN_ADDR(addr) (void *)( ((PMM_FRAME_SIZE-1) + (ulong)(addr)) & ~(PMM_FRAME_SIZE-1) )

/*
 * Calculate how much memory is needed by the PMM for it to keep track of free/alloced frames.
 *
 * heapSize - the size (in # of bytes) of the heap from which frames can be allocated..
 *          - Note, that this API will have to change once frames can be swapped out to disk, as in that case there will be no max number of frames. Once this happens, the pmm will then need to be modified so that is allocates/maps memory as it needs it to keep track of frames.
 *
 * returns - how many bytes are need by the PMM.
 */
size_t pmm_mem_reqs(size_t heapSize);

/**
 * Initialize the physical memory manager.
 *
 * memheap - physical address to the start of the heap, as determined by the kernel initialization
 * pmmstart - the memory space the pmm can use to keep track of free/alloced frames (virtual address if MMU enabled). Must be at least pmm_mem_reqs() in size.
 */
void pmm_init(void* memheap, void* pmmstart);

/**
 * Allocate one frame.
 *
 * return - address to a new frame, or NULL if out of memory.
 */
intptr_t pmm_alloc_frame(void);

/**
 * Free a previously allocated frame.
 */
void pmm_free_frame(intptr_t frame);


