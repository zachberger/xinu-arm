/**
 * Xinu Physical Memory Manager/Frame Allocator
 *
 * Author: Jedd Haberstro
 * Date: March 26, 2013
 **/

#ifndef PMM_PMM_H
#define PMM_PMM_H

#include <stdint.h>
#include <stdbool.h>

#define PMM_FRAME_SIZE 4096

#define PMM_ALIGN_ADDR(addr) (void *)( ((PMM_FRAME_SIZE-1) + (ulong)(addr)) & ~(PMM_FRAME_SIZE-1) )


typedef struct pmm_t pmm_t;

struct pmm_t {
    uint32_t* bitmap;            // Start of the bitmap
    uint8_t* heapStart;		     // Start of the heap from which to allocate frames
    size_t memReqs;              // Size of the memory region (in # of bytes) that the pmm can
                                 //  use for keeping track of frames.
    size_t firstFreeFrameIndex;  // Keeps track of where the first unallocated frame is located
                                 //  to alleviate the O(n) allocation time.
};

/*
 * Calculate how much memory is needed by the PMM for it to keep track of free/alloced frames.
 *
 * heapSize - the size (in # of bytes) of the heap from which frames can be allocated..
 *
 * returns - how many bytes are need by the PMM.
 */
size_t pmm_mem_reqs(pmm_t* pmm, uint64_t heapSize);

/**
 * Initialize the physical memory manager.
 *
 * memheap - physical address to the start of the heap, as determined by the kernel initialization
 * pmmstart - the memory space the pmm can use to keep track of free/alloced frames (virtual address if MMU enabled). Must be at least pmm_mem_reqs() in size.
 */
void pmm_init(pmm_t* pmm, void* memheap, void* pmmstart);

/**
 * Allocate one frame.
 *
 * return - physical address to a new frame, or NULL if out of memory.
 */
uintptr_t pmm_alloc_frame(pmm_t* pmm, bool* out_failed);

/**
 * Allocate a contigious set of frames.
 *
 * return - the physical address to the start of the contigious frames, or NULL if unable to find such a set.
 */
uintptr_t pmm_alloc_frames(pmm_t* pmm, size_t num_pages, bool* out_failed);

/**
 * Free a previously allocated frame.
 */
void pmm_free_frame(pmm_t* pmm, uintptr_t frame);

#endif // PMM_PMM_H

