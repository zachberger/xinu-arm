//Eric Warrington

//get fb
//query for amt of mem
//allocate, free GPU mem
//execute code on GPU

#include "mbox.h"
#include "mailbox.h"

/* use a global variable since we have no malloc */
static struct fb_info FB;

/* Use some free memory in the area below the kernel/stack */
#define BUFFER_ADDRESS 0x1000

static void fb_error()
{
    //while(1)
    {
        printf("FB Error\n");
        //led_flash_num(num, FB_ERROR_MAX_BITS);
    }
}

void get_display_size(struct fb_info* info);
void get_fb_info(struct fb_info* info);
struct fb_info* fb_init()
{
    struct fb_info* info = &FB;
    get_display_size(info);
    get_fb_info(info);
    return info;
}

void get_arm_memory(u32* addr, u32* size)
{
    volatile u32* buf=(u32*)BUFFER_ADDRESS;
    //u32 addr, size;

    buf[0] = 8 * 4; /* buffer size */
    buf[1] = 0; /* request/response code */

    /* tag - get physical display size*/
    buf[2] = 0x00010005; /* tag id */
    buf[3] = 8; /* value buffer size */
    buf[4] = 0+0; /* request indicator + size */
    buf[5] = 0; /* req: mem address */
    buf[6] = 0; /* req: mem size */

    buf[7] = 0; /* end tags */

    mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);
    mailbox_read(MAIL_CHAN_ARMTOVC_PROP);

    if(buf[1]!=MAIL_PROP_RESPONSE_OK)
    {
        fb_error();
        return;
    }

    *addr=buf[5];
    *size=buf[6];

    /*if(!addr && !size)
    {
        w = 640;
        h = 480;
    }*

    if(!size || !addr)
    {
        fb_error(FB_ERR_BAD_RESOLUTION);
        return;
    }*/

    //info->width = w;
    //info->height = h;
}

void get_vc_memory(u32* addr, u32* size)
{
    volatile u32* buf=(u32*)BUFFER_ADDRESS;
    //u32 addr, size;

    buf[0] = 8 * 4; /* buffer size */
    buf[1] = 0; /* request/response code */

    /* tag - get physical display size*/
    buf[2] = 0x00010006; /* tag id */
    buf[3] = 8; /* value buffer size */
    buf[4] = 0+0; /* request indicator + size */
    buf[5] = 0; /* req: mem address */
    buf[6] = 0; /* req: mem size */

    buf[7] = 0; /* end tags */

    mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);
    mailbox_read(MAIL_CHAN_ARMTOVC_PROP);

    if(buf[1]!=MAIL_PROP_RESPONSE_OK)
    {
        fb_error();
        return;
    }

    *addr=buf[5];
    *size=buf[6];

    /*if(!addr && !size)
    {
        w = 640;
        h = 480;
    }*

    if(!size || !addr)
    {
        fb_error(FB_ERR_BAD_RESOLUTION);
        return;
    }*/

    //info->width = w;
    //info->height = h;
}

#define MEM_FLAG_ZERO = 1 << 4  /* initialize buffer to all zeros */
u32 gpu_alloc(u32 size, u32 alignment)
{
    volatile u32* buf=(u32*)BUFFER_ADDRESS;
    //u32 addr, size;

    buf[0] = 9 * 4; /* buffer size */
    buf[1] = 0; /* request/response code */

    /* tag - Allocate Memory*/
    buf[2] = 0x0003000C; /* tag id */
    buf[3] = 12; /* value buffer size */
    buf[4] = 0+12; /* request indicator + size */
    buf[5] = size;		// req: amt of mem to alloc
    buf[6] = alignment;	// req: alignment
    buf[7] = (1 << 4);	/* req: flags */
    buf[8] = 0; /* end tags */

    mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);
    mailbox_read(MAIL_CHAN_ARMTOVC_PROP);

    if(buf[1]!=MAIL_PROP_RESPONSE_OK)
    {
        fb_error();
        return;
    }

    return buf[5];	//handle
}

u32 gpu_free(u32 handle)
{
    volatile u32* buf=(u32*)BUFFER_ADDRESS;
    //u32 addr, size;

    buf[0] = 7 * 4; /* buffer size */
    buf[1] = 0; /* request/response code */

    /* tag - Release Memory*/
    buf[2] = 0x0003000F; /* tag id */
    buf[3] = 4; /* value buffer size */
    buf[4] = 0+4; /* request indicator + size */
    buf[5] = handle;		/* req: handle */
    buf[6] = 0; /* end tags */

    mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);
    mailbox_read(MAIL_CHAN_ARMTOVC_PROP);

    if(buf[1]!=MAIL_PROP_RESPONSE_OK)
    {
        fb_error();
        return;
    }

    return buf[5];	//status
}


/**
 * Calls the function at given (bus) address and with arguments given.
 *		i.e. r0 = fn(r0, r1, r2, r3, r4, r5)
 * It blocks until call completes. The (GPU) instruction cache is implicitly flushed.
 * Setting the lsb of function pointer address will suppress the instruction cache
 * flush if you know the buffer hasn't changed since last execution.
 */
u32 gpu_execute(u32 fnptr, u32 r0, u32 r1, u32 r2, u32 r3, u32 r4, u32 r5)
{
    volatile u32* buf=(u32*)BUFFER_ADDRESS;
    //u32 addr, size;

    buf[0] = 12 * 4; /* buffer size */
    buf[1] = 0; /* request/response code */

    /* tag - Execute Code*/
    buf[2] = 0x30010; /* tag id */
    buf[3] = 28;	/* value buffer size */
    buf[4] = 0+28;	/* request indicator + size */
    buf[5] = fnptr;	/* req: function pointer */
    buf[6] = r0;	/* req: r0 */
    buf[7] = r1;	/* req: r1 */
    buf[8] = r2;	/* req: r2 */
    buf[9] = r3;	/* req: r3 */
    buf[10] = r4;	/* req: r4 */
    buf[11] = r5;	/* req: r5 */
    
    buf[12] = 0; /* end tags */

    mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);
    mailbox_read(MAIL_CHAN_ARMTOVC_PROP);

    if(buf[1]!=MAIL_PROP_RESPONSE_OK)
    {
        fb_error();
        return;
    }

    return buf[6];	//r0
}


void get_display_size(struct fb_info* info)
{
    volatile u32* buf=(u32*)BUFFER_ADDRESS;
    int w, h;

    buf[0] = 8 * 4; /* buffer size */
    buf[1] = 0; /* request/response code */

    /* tag - get physical display size*/
    buf[2] = 0x40003; /* tag id */
    buf[3] = 8; /* value buffer size */
    buf[4] = 0+0; /* request indicator + size */
    buf[5] = 0; /* req: width */
    buf[6] = 0; /* req: height */

    buf[7] = 0; /* end tags */

    mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);
    mailbox_read(MAIL_CHAN_ARMTOVC_PROP);

    if(buf[1]!=MAIL_PROP_RESPONSE_OK)
    {
        fb_error();
        return;
    }
    w = buf[5];
    h = buf[6];

    if(!w && !h)
    {
        w = 640;
        h = 480;
    }

    if(!w || !h)
    {
        fb_error();
        return;
    }
    info->width = w;
    info->height = h;

    buf[2] = 0x00040008;
    buf[3] = 4;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);
    mailbox_read(MAIL_CHAN_ARMTOVC_PROP);

    if(buf[1]!=MAIL_PROP_RESPONSE_OK)
    {
        fb_error();
        return;
    }
    info->pitch = buf[5];

}

void get_fb_info(struct fb_info* info)
{
    volatile u32* buf=(u32*)BUFFER_ADDRESS;
    u32 w = info->width;
    u32 h = info->height;
    u32 i = 0;
    u32 buf_size = 0;
    u32 fb_base;
    u32 fb_size;

    buf[i++] = 0; /* buffer size; compute later */
    buf[i++] = 0; /* request/response code */

    /* tag - set physical size */
    buf[i++] = 0x48003; /* tag id */
    buf[i++] = 8; /* value buffer size */
    buf[i++] = 0+8; /* request indicator + size */
    buf[i++] = w; /* req: width */
    buf[i++] = h; /* req: height */

    /* tag - set virtual size */
    buf[i++] = 0x48004; /* tag id */
    buf[i++] = 8; /* value buffer size */
    buf[i++] = 0+8; /* request indicator + size */
    buf[i++] = w; /* req: width */
    buf[i++] = h; /* req: height */

    /* tag - set depth */
    buf[i++] = 0x48005; /* tag id */
    buf[i++] = 4; /* value buffer size */
    buf[i++] = 0+4; /* request indicator + size */
    buf[i++] = 16; /* req: depth */

    /* tag - allocate framebuffer */
    buf[i++] = 0x40001; /* tag id */
    buf[i++] = 8; /* value buffer size */
    buf[i++] = 0+4; /* request indicator + size */
    buf[i++] = 16; /* req: alignment; resp: fb base addr */
    buf[i++] = 0; /* resp: fb size */

    buf[i++] = 0; /* end tags */

    buf_size=i;
    buf[0] = buf_size * 4; /* buffer size */

    mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);
    mailbox_read(MAIL_CHAN_ARMTOVC_PROP);

    if(buf[1]!=MAIL_PROP_RESPONSE_OK)
    {
        fb_error();
        return;
    }

    /* jump to the last tag since we're only interested in this one */
    i = 2; /* first tag */
    while(buf[i]!=0x40001)
    {
        /* skip to next tag */
        i += 3 + (buf[i+1] >> 2);
        if(i >= buf_size)
        {
            fb_error();
            return;
        }
    }

    fb_base = buf[i+3];
    fb_size = buf[i+4];
    if(!fb_base || !fb_size)
    {
        fb_error();
        return;
    }

    info->fb_ptr=fb_base;
    info->fb_size=fb_size;
}
