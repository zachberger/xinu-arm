/**
 * Xinu Physical Memory Manager/Frame Allocator implementation.
 *
 * Author: Jedd Haberstro
 * Date: March 26, 2013
 *
 * The PMM employs a bitmap vector to keep track of alloced/free frames.
 * Allocated is O(n), deallocation is O(1), and the memory requirement is small (1 byte per 8 frame)
 * A 0 bit signifies that the frame has not been allocated, and (obviously) a 1 bit signifies that the frame has been allocated.
 *
 * TODO - Testing!
 **/

#include <pmm.h>
#include <stdint.h>
#include <stddef.h>

static uint32_t* _bitmap = NULL;            // Start of the bitmap
static uint8_t* _alignedHeapStart = NULL;   // Start of the heap from which to allocate frames
static size_t _maxFrames = 0;               // Maximum possible of frames that can be allocated
static size_t _memReqs = 0;                 // Size of the memory region (in # of bytes) that the pmm can use for keeping track of frames.
static size_t _firstFreeFrameIndex = 0;     // Keep track of where the first unallocated frame is located to alleviate the O(n) allocation time.

size_t pmm_mem_reqs(size_t heapSize) {
    _maxFrames = heapSize / PMM_FRAME_SIZE;

    // With a bitmap, one byte can keep track of 8 frames
    _memReqs = _maxFrames / 8; 
    return _memReqs;
}

void pmm_init(void* memheap, void* pmmstart) {
    _alignedHeapStart = PMM_ALIGN_ADDR((intptr_t)memheap);
    _bitmap = pmmstart;

    // Zero out the bitmap
    size_t i;
    for (i = 0; i < (_memReqs / sizeof(_bitmap)); ++i) {
        _bitmap[i] = 0;
    }
}

intptr_t pmm_alloc_frame() {
    size_t i;
    size_t bit;

    size_t entrySize = sizeof(_bitmap);
    size_t numberOfBitmapEntries = _memReqs / entrySize;
    for (i = _firstFreeFrameIndex; i < numberOfBitmapEntries; ++i) {
        uint32_t entry = _bitmap[i];
        if (entry != 0xFF) {
            for (bit = 0; bit < (entrySize * 8); ++bit) {
                if ((entry & (1 << bit)) == 0) {
                    _firstFreeFrameIndex = i;
                    _bitmap[i] = entry | (1 << bit);
                    return (intptr_t)(_alignedHeapStart + (((i * entrySize * 8) + bit) * PMM_FRAME_SIZE));
                }
            }
        }
    }

    return NULL;
}

void pmm_free_frame(intptr_t frame) {
    intptr_t absoluteAddress = frame - ((intptr_t)_alignedHeapStart); 
    int frameNum = absoluteAddress / PMM_FRAME_SIZE;
    int numBitsInEntry = sizeof(_bitmap) * 8;
    int bitmapIndex = frameNum / numBitsInEntry;
    int bit = frameNum % numBitsInEntry;
    _bitmap[bitmapIndex] &= ~(1 << bit);
    
    if (bitmapIndex < _firstFreeFrameIndex) {
        _firstFreeFrameIndex = bitmapIndex;
    }
}

