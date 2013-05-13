//Eric Warrington

#ifndef GPUCODE_H
#define GPUCODE_H

#include "mbox.h"

/* Use some free memory in the area below the kernel/stack */
#define BUFFER_ADDRESS 0x1000

u32 gpu_alloc(u32 size, u32 alignment);
u32 gpu_free(u32 handle);
u32 gpu_execute();

#endif
