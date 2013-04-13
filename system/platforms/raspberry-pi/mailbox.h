//Eric Warrington

#ifndef MAILBOX_H
#define MAILBOX_H

#include "mbox.h"

/* Use some free memory in the area below the kernel/stack */
#define BUFFER_ADDRESS 0x1000
struct fb_info
{
    u32 width;
    u32 height;
    u32 pitch;
    u32 fb_ptr;
    u32 fb_size;
} __attribute__ ((aligned(16)));


struct fb_info* fb_init();

void get_arm_memory(u32* addr, u32* size);
void get_vc_memory(u32* addr, u32* size);

u32 gpu_alloc(u32 size, u32 alignment);
u32 gpu_free(u32 handle);
u32 gpu_execute();

#endif
