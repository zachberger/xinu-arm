/**
 * Xinu Physical Memory Manager/Frame Allocator implementation.
 *
 * Author: Jedd Haberstro
 * Date: March 26, 2013
 *
 * The PMM employs a bitmap vector to keep track of alloced/free frames.
 * Allocation is O(n), deallocation is O(1), and the memory requirement is small (1 byte per 8 frames)
 * A 0 bit signifies that the frame has not been allocated, and (obviously) a 1 bit signifies that the frame has been allocated.
 *
 * TODO - Testing!
 **/

#include <pmm.h>
#include <stdint.h>
#include <stddef.h>

struct pmm_t {
    uint32_t* bitmap;            // Start of the bitmap
    uint8_t* alignedHeapStart;   // Start of the heap from which to allocate frames
    size_t memReqs;              // Size of the memory region (in # of bytes) that the pmm can
                                 //  use for keeping track of frames.
    size_t firstFreeFrameIndex;  // Keeps track of where the first unallocated frame is located
                                 //  to alleviate the O(n) allocation time.
};

size_t pmm_mem_reqs(pmm_t* pmm, size_t heapSize) {
    size_t maxFrames = heapSize / PMM_FRAME_SIZE;

    // With a bitmap, one byte can keep track of 8 frames
    pmm->memReqs = maxFrames / 8; 
    return pmm->memReqs;
}

void pmm_init(pmm_t* pmm, void* memheap, void* pmmstart) {
    pmm->alignedHeapStart = PMM_ALIGN_ADDR((uintptr_t)memheap);
    pmm->bitmap = pmmstart;

    // Zero out the bitmap
    size_t i;
    for (i = 0; i < (pmm->memReqs / sizeof(pmm->bitmap)); ++i) {
        pmm->bitmap[i] = 0;
    }
}

uintptr_t pmm_alloc_frame(pmm_t* pmm) {
    size_t i;
    size_t bit;

    size_t entrySize = sizeof(pmm->bitmap);
    size_t numberOfBitmapEntries = pmm->memReqs / entrySize;
    for (i = pmm->firstFreeFrameIndex; i < numberOfBitmapEntries; ++i) {
        uint32_t entry = pmm->bitmap[i];

        // I want to compare entry to the MAX_VAL(typeof(entry)), but we don't have that constant, so I just flip the bits and compare to 0.
        if ((~entry) != 0) {
            for (bit = 0; bit < (entrySize * 8); ++bit) {
                if ((entry & (1 << bit)) == 0) {
                    pmm->firstFreeFrameIndex = i;
                    pmm->bitmap[i] = entry | (1 << bit);
                    return (uintptr_t)(pmm->alignedHeapStart + (((i * entrySize * 8) + bit) * PMM_FRAME_SIZE));
                }
            }
        }
    }

    return NULL;
}

void pmm_free_frame(pmm_t* pmm, uintptr_t frame) {
    uintptr_t absoluteAddress = frame - ((uintptr_t)pmm->alignedHeapStart); 
    int frameNum = absoluteAddress / PMM_FRAME_SIZE;
    int numBitsInEntry = sizeof(pmm->bitmap) * 8;
    int bitmapIndex = frameNum / numBitsInEntry;
    int bit = frameNum % numBitsInEntry;
    pmm->bitmap[bitmapIndex] &= ~(1 << bit);
    
    if (bitmapIndex < pmm->firstFreeFrameIndex) {
        pmm->firstFreeFrameIndex = bitmapIndex;
    }
}

