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
    int i, count, addr_offset, page_num, table_num;
    volatile paddr_t phys_base_addr;
    volatile vaddr_t vbase_addr;

    ptr = (coarse_t*) pd;
    count = size / 4096; // 256 obv
    addr_offset = 4096;
    phys_base_addr = phys; // 1:1 for now, change this later

    for (i = 0; i < count; ++i) {
        vbase_addr = virt + i * 4096;
        table_num = vbase_addr / 0x100000;
        page_num = (vbase_addr & 0xFF000) >> 12;

        entry = phys_base_addr & 0xFFFFF000;
        entry |= 0x11; // small page number
        entry |= 0x30;// AP = 3

        (*(ptr + table_num)).small[page_num] = entry;

        phys_base_addr += addr_offset;
    }
}

void set_page_mapping(pde_t *pd, vaddr_t virt, paddr_t phys) {
    // REMOVE{
    pd[virt >> 20] |= phys & 0xFFF00000;
    pd[virt >> 20] |= 0x02; // section entry number
    pd[virt >> 20] |= 0x10; // execute never
    pd[virt >> 20] |= 0xC00; // AP = 3
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
        };
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

    //static l1_table master __attribute__ ((aligned(0x4000)));
    //static l2_table tables[4096] __attribute__ ((aligned(0x4000)));

    /*int coarse, entry;
    intptr_t currentAddress = 0x0;
    for (coarse = 0; coarse < 4096; ++coarse) {
        l2_table *l2 = &tables[i];
        master.descriptors[coarse].start = 1;
        master.descriptors[coarse].ns = 0;
        master.descriptors[coarse].domain = 0;
        master.descriptors[coarse].p = 0;
        master.descriptors[coarse].addr = ((intptr_t)l2) >> 10;
        for (entry = 0; entry < 256; ++entry) {
            l2->ptes[entry].xn = 1;  
            l2->ptes[entry].one = 1;  
            l2->ptes[entry].b = 0;  
            l2->ptes[entry].c = 0;  
            l2->ptes[entry].ap = 0;  // kernel read/write ? 
            l2->ptes[entry].tex = 0;  
            l2->ptes[entry].apx = 0;  
            l2->ptes[entry].s = 0;
            l2->ptes[entry].ng = 0;  
            l2->ptes[entry].addr = ((intptr_t)l2) >> 12;
            currentAddress += 0x4000;
        }
    }*/

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

    static pde_t master[MAX_SIZE] __attribute__ ((aligned(0x4000)));
    static coarse_t l2[MAX_SIZE] __attribute__ ((aligned(0x400)));
    volatile pde_t entry;
    int j;

    // REMOVE: 1:1 section table
    //for (j = 0; j < MAX_SIZE; ++j) {
    //    set_page_mapping(master, j << 20, j << 20);
    //}

    for (j = 0; j < MAX_SIZE; ++j) {
        entry = (((uint)l2 + (uint)j*1024) & 0xFFFFFC00);
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
