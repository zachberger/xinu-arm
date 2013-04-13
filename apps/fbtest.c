#include <stdio.h>
#include "../system/platforms/raspberry-pi/mailbox.h"
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;


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
