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

extern ulong cpuid;                     /* Processor id                    */
extern struct platform platform;        /* Platform specific configuration */

//NOTE on the Raspberry Pi, we rely on the GPU to have initialized the PL011
//properly so that we can do kprintf since we don't actually initialize it
//until the latter part of sysinit()

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

    typedef struct __attribute__ ((__packed__)) {
        union {
            struct {
                unsigned start  : 3;
                unsigned ns     : 2;
                unsigned domain : 4;
                unsigned p      : 1;
                unsigned addr   : 22;
            };
            uint32_t value;
        }
    } coarse_descriptor;

    typedef struct __attribute__ ((__packed__)) {
        unsigned xn   : 1;
        unsigned one  : 1;
        unsigned b    : 1;
        unsigned c    : 1;
        unsigned ap   : 2;
        unsigned tex  : 3;
        unsigned apx  : 1;
        unsigned s    : 1;
        unsigned ng   : 1;
        unsigned addr : 20;
    } small_page_descriptor;

    typedef struct {
        coarse_descriptor descriptors[4096];
    } l1_table;

    typedef struct {
        small_page_descriptor ptes[256];
    } l2_table;

    static l1_table master __attribute__ ((aligned(0x4000)));
    static l2_table tables[512] __attribute__ ((aligned(0x4000)));
    int coarse, entry;
    intptr_t currentAddress = 0x0;
    for (coarse = 0; coarse < 512; ++coarse) {
        l2_table* l2 = &tables[i];
        master.descriptors[coarse].start = 1;
        master.descriptors[coarse].ns = 1;
        master.descriptors[coarse].domain = 0;
        master.descriptors[coarse].p = 1;
        master.descriptors[coarse].addr = ((intptr_t)l2) >> 10;
        for (entry = 0; entry < 256; ++entry) {
            l2->ptes[256].xn = 1;  
            l2->ptes[256].one = 1;  
            l2->ptes[256].b = 0;  
            l2->ptes[256].c = 0;  
            l2->ptes[256].ap = 3;  // kernel read/write ? 
            l2->ptes[256].tex = 0;  
            l2->ptes[256].apx = 0;  
            l2->ptes[256].s = 0;
            l2->ptes[256].ng = 0;  
            l2->ptes[256].addr = currentAddress >> 12;
            currentAddress += 0x4000;
        }
    }

    //static struct __attribute__ ((__packed__)) {
    //    struct __attribute__ ((__packed__)) {
    //        unsigned zero : 1;
    //        unsigned one  : 1;
    //        unsigned b    : 1;
    //        unsigned c    : 1;
    //        unsigned xn   : 1;
    //        unsigned ign0 : 4;
    //        unsigned imp  : 1;
    //        unsigned ap   : 2;
    //        unsigned tex  : 3;
    //        unsigned apx  : 1;
    //        unsigned s    : 1;
    //        unsigned nG   : 1;
    //        unsigned zero2: 1;
    //        unsigned ns   : 1;
    //        unsigned base : 12;
    //    } entries[4096];
    //} tlb_l1 __attribute__ ((aligned(0x4000))); // 16 KB aligned

    //for (i = 0; i < 4096; ++i) {
    //    tlb_l1.entries[i].zero = 0; //
    //    tlb_l1.entries[i].one = 1;  //
    //    tlb_l1.entries[i].b = 0;    //
    //    tlb_l1.entries[i].c = 0;    //
    //    tlb_l1.entries[i].xn = 1;   //
    //    tlb_l1.entries[i].ign0 = 0; // 
    //    tlb_l1.entries[i].imp = 0;  // 
    //    tlb_l1.entries[i].ap = 0;   //
    //    tlb_l1.entries[i].tex = 0;  // 
    //    tlb_l1.entries[i].apx = 0;  //
    //    tlb_l1.entries[i].s = 0;    //
    //    tlb_l1.entries[i].nG = 0;   // 
    //    tlb_l1.entries[i].zero2 = 0;//
    //    tlb_l1.entries[i].ns = 0;   //
    //    tlb_l1.entries[i].base = i;
    //}
    
    uint tlb_l1_base = (uint)&master;
    asm("mcr p15, 0, %0, c2, c0, 0" : : "r" (tlb_l1_base));

    // section table entry
    //uint dest_addr = tlb_l1_base | (0x080 * 4);
    //uint entry = tlb_l1_base | 0xc02;
    //*(uint*)(dest_addr) = entry;

    // domain AP: "manager" (AP not checked)
    asm("mov r0, #0x3");
    asm("mcr p15, 0, r0, c3, c0, 0");

    // enable
    asm("mrc p15, 0, r0, c1, c0, 0");
    asm("orr r0, r0, #0x1");
    asm("mcr p15, 0, r0, c1, c0, 0");


    return OK;
}
