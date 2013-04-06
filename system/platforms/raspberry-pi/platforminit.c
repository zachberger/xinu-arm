/**
 * @file platforminit.c
 * @provides platforminit.
 */

#include <kernel.h>
#include <platform.h>
#include <uart.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "vic.h"
#include "gpio.h"

#define MAX_SIZE 4096

typedef unsigned long vaddr_t;
typedef unsigned long paddr_t;
typedef unsigned long pde_t;

typedef struct coarse {
    pde_t small[256];
} coarse_t;

extern ulong cpuid;                     /* Processor id                    */
extern struct platform platform;        /* Platform specific configuration */

//NOTE on the Raspberry Pi, we rely on the GPU to have initialized the PL011
//properly so that we can do kprintf since we don't actually initialize it
//until the latter part of sysinit()

void dump_table(pde_t *table, int start, int end) {
    int i;
    for (i = start; i < end; ++i) {
        kprintf("%0d: %x\r\n", i, table[i]);
    }
}

void init_table(coarse_t *pd, vaddr_t virt, paddr_t phys, int size) {
    volatile coarse_t *ptr; 
    volatile pde_t entry;
    int i, count, addr_increment, page_num, table_num;
    volatile paddr_t phys_curr_addr;
    volatile vaddr_t virt_curr_addr;

    ptr = (coarse_t*) pd;
    count = size / 4096; // 256 obv
    addr_increment = 4096;
    phys_curr_addr = phys; // 1:1 for now, change this later
    virt_curr_addr = virt;

    for (i = 0; i < count; ++i) {
        // A course page table can map 1 MB, so
        // divide by 1 MB to calculate the correct table
        table_num = virt_curr_addr / 0x100000;
        page_num = (virt_curr_addr & 0xFF000) >> 12;

        entry = phys_curr_addr & 0xFFFFF000;
        entry |= 0x11; // small page number
        entry |= 0x30;// AP = 3

        (*(ptr + table_num)).small[page_num] = entry;

        // increment by 4kb
        phys_curr_addr += addr_increment;
        virt_curr_addr += addr_increment;
    }
}

/**
 * Determines and stores all platform specific information.
 * @return OK if everything is determined successfully
 */
int platforminit( void )
{
    unsigned char c;
    int i;

    //set GPIO pin 14 to PL011 UART TX mode
    GPIOMODE(14, FSEL_AF0);
    //set GPIO pin 15 to PL011 UART RX mode
    GPIOMODE(15, FSEL_AF0);
    //turn off pull-up/pull-down resistors on the TX and RX pins
    GPIOPULLOF(14);
    GPIOPULLOF(15);

    //initialize the VIC
    vic_init();

    /*
     * Go into an infinite loop waiting for user to type the @ sign.
     * This will get Xinu going and produce output, like "Welcome
     * to Xinu."
     */

    i = 0;
    while (1) {
        if (i==0) {
            kprintf("Press @ to begin.\r\n");
        }
        i = (i+1)%100000;
        c = kgetc(&devtab[SERIAL0]);
        if (c == '@') {
            break;
        }
    }

    //fill in the platform information struct (from include/platform.h)
    strncpy(platform.name, "Raspberry Pi", PLT_STRMAX);
    strncpy(platform.family, "ARM", PLT_STRMAX);
    platform.maxaddr = (void *)(0x8000000/*128MB of RAM*/) /** \todo dynamically determine? */;
    platform.clkfreq = 1000000 /*we have a 1Mhz input clock to the system timer*/ /** \todo dynamically determine? */;
    //platform.uart_dll = 1337 /*Divisor Latch Low Byte, not useful?*/ /** \todo fixme */;
    //platform.uart_irqnum = 0; /*UART IRQ number? Not read anywhere.*/

    static coarse_t l2[MAX_SIZE];// __attribute__ ((aligned(0x400)));
    static pde_t master[MAX_SIZE] __attribute__ ((aligned(0x4000)));
    volatile pde_t entry;
    int j;

    for (j = 0; j < MAX_SIZE; ++j) {
        entry = ((uint)&l2[j]) & 0xFFFFFC00;
        entry |= 0x1;
        init_table(l2, j << 20, j << 20, 0x100000);
        master[j] = entry;
    }

    // REMOVE: debug
    dump_table(master, 0, 20);
    
    // REMOVE: coarse table at first 1 MB of master
    //init_table(l2, 0, 0, 0x100000);
    //master[0] = ((uint)l2 & 0xFFFFFC00);
    //master[0] |= 0x1;

    //uint tlb_l1_base = (uint)&master;
    asm("mcr p15, 0, %0, c2, c0, 0" : : "r" (master) : "memory");

    // domain AP: "manager" (AP not checked)
    asm("mov r0, #0x3");
    asm("mcr p15, 0, r0, c3, c0, 0");

    // enable
    asm("mrc p15, 0, r0, c1, c0, 0");
    asm("orr r0, r0, #0x1");
    asm("mcr p15, 0, r0, c1, c0, 0");
    
    return OK;
}
