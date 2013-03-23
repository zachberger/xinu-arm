#include <stdio.h>

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define MAILBOX_ADDR_READ 0x2000B880
#define MAILBOX_ADDR_POLL 0x2000B890
#define MAILBOX_ADDR_SENDER 0x2000B894
#define MAILBOX_ADDR_STATUS 0x2000B898
#define MAILBOX_ADDR_CONFIG 0x2000B89C
#define MAILBOX_ADDR_WRITE 0x2000B8A0

/* bit in status register */
#define MAILBOX_FULL 0x80000000
#define MAILBOX_EMPTY 0x40000000

/* channels */
#define MAIL_CHANNEL_FB 1
#define MAIL_CHANNEL_PROPERTY 8

/* property channel */
#define MAIL_PROPERTY_RESPONSE_OK 0x80000000

u32 mailbox_read(u8 chan);
int mailbox_write(u8 chan, u32 val);

static u32 mailbox_status()
{
    return *(u32 *)MAILBOX_ADDR_STATUS;
}

/* return 1 if mailbox is empty */
static u32 mailbox_empty()
{
    return mailbox_status() & MAILBOX_EMPTY;
}

/* return 1 if mailbox is full */
static u32 mailbox_full()
{
    return mailbox_status() & MAILBOX_FULL;
}

/* return read value
 * * return 0 on error
 * */
u32 mailbox_read(u8 chan)
{
    u32 val;

    /* channel: 4 bits */
    if (chan > 15) {
        return 0;
    }

    while (1) {
        while (mailbox_empty()) { ; }
        
        val = *(u32 *)MAILBOX_ADDR_READ;
        if ((val & 0b1111) == chan) {
            return val & ~0b1111;
        }
    }
}

/* return status; 0 on error */
int mailbox_write(u8 chan, u32 val)
{
    /* channel: 4 bits
     * * val: lowest 4 bits must be 0
     * */
    if (chan > 15 || val & 0b1111) {
        return 0;
    }

    while (mailbox_full()) { ; }

    *(u32 *)MAILBOX_ADDR_WRITE = val | chan;

    return 1;
}


#define FB_ERR_GET_RESOLUTION 1
#define FB_ERR_BAD_RESOLUTION 2
#define FB_ERR_SETUP_SCREEN 3
#define FB_ERR_BAD_FB 4

#define FB_ERR_PROGRAMMER 5
#define FB_ERROR_MAX_BITS 3

/* use a global variable since we have no malloc */
struct fb_info {
    u32 width;
    u32 height;
    u32 fb_ptr;
    u32 fb_size;
} __attribute__ ((aligned(16))); /* why? */

/* use a global variable since we have no malloc */
static struct fb_info FB;

/* Use some free memory in the area below the kernel/stack */
#define BUFFER_ADDRESS 0x1000

static void fb_error(u32 num)
{
    while (1) {
        printf("fb_error\n");
        //led_flash_num(num, FB_ERROR_MAX_BITS);
    }
}

static void get_display_size(struct fb_info *info)
{
    volatile u32 *buf = (u32 *)BUFFER_ADDRESS;
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

    mailbox_write(MAIL_CHANNEL_PROPERTY, (u32)buf);

    mailbox_read(MAIL_CHANNEL_PROPERTY);

    if (buf[1] != MAIL_PROPERTY_RESPONSE_OK) {
        fb_error(FB_ERR_GET_RESOLUTION);
        return;
    }

    w = buf[5];
    h = buf[6];

    if (!w && !h) {
        w = 640;
        h = 480;
    }

    if (!w || !h) {
        fb_error(FB_ERR_BAD_RESOLUTION);
        return;
    }

    info->width = w;
    info->height = h;
}

void setup_screen(struct fb_info *info)
{
    volatile u32 *buf = (u32 *)BUFFER_ADDRESS;
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

    buf_size = i;
    buf[0] = buf_size * 4; /* buffer size */

    mailbox_write(MAIL_CHANNEL_PROPERTY, (u32)buf);
    mailbox_read(MAIL_CHANNEL_PROPERTY);

    if (buf[1] != MAIL_PROPERTY_RESPONSE_OK) {
        fb_error(FB_ERR_SETUP_SCREEN);
        return;
    }

    /* jump to the last tag since we're only interested in this one */
    i = 2; /* first tag */
    while (buf[i] != 0x40001) {
        /* skip to next tag */
        i += 3 + (buf[i+1] >> 2);
        if (i >= buf_size) {
            fb_error(FB_ERR_PROGRAMMER);
            return;
        }
    }

    fb_base = buf[i+3];
    fb_size = buf[i+4];
    if (!fb_base || !fb_size) {
        fb_error(FB_ERR_BAD_FB);
        return;
    }

    info->fb_ptr = fb_base;
    info->fb_size = fb_size;
}

void next_color(u8 *r, u8 *g, u8 *b)
{
    /*
     * http://www.controlbooth.com/attachments/lighting-electrics/8328d1355455375-color-mixing-seven-colors-make-white-rgb-color-wheel-lg.jpg
     * */
    /* red to yellow; increasing green */
    if (*r == 31 && *b == 0 && *g < 31) {
        (*g)++;
    /* yellow to green; decreasing red */
    } else if (*g == 31 && *b == 0 && *r > 0) {
        (*r)--;
    /* green to cyan; increasing blue */
    } else if (*r == 0 && *g == 31 && *b < 31) {
        (*b)++;
    /* cyan to blue; decreasing green */
    } else if (*r == 0 && *b == 31 && *g > 0) {
        (*g)--;
    /* blue to violet; increasing red */
    } else if (*g == 0 && *b == 31 && *r < 31) {
        (*r)++;
    /* violet to red; decreasing blue */
    } else if (*r == 31 && *g == 0 && *b > 0) {
        (*b)--;
    }
}

struct fb_info *fb_init()
{
    struct fb_info *info = &FB;
    get_display_size(info);
    setup_screen(info);
    return info;
}

void fbtest() {
    struct fb_info* fb = fb_init();
    if (!fb) {
        printf("fb null\n");
        return;
    }

    u32 i, j;
    u16 color;
    u16* addr;
    
    u8 r0, g0, b0;
    u8 r, g, b;
    r0 = 31; g0 = 0; b0 = 0; /* begin with red */
    addr = (u16 *)fb->fb_ptr;
    for (j = 0; j < fb->height; j++) {
        r = r0; g = g0; b = b0;
        for (i = 0; i < fb->width; i++) {
            color = (r << 11) | (g << 5) | b;
            *addr++ = color;
            next_color(&r, &g, &b);
        }
        next_color(&r0, &g0, &b0);
    }
}
