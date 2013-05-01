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
#include "mmu.h"

static vmm_t vmm;
static pmm_t pmm;

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
    
    kprintf("Starting MMU initialization\r\n");
    size_t const kPagesIn4GB = 1048576/2;
    uint64_t const kBytesin4GB = kPagesIn4GB * 4096ull;
//  kprintf("kBytesin4GB: %d\r\n", kBytesin4GB);
    size_t vmmStorageSize = pmm_mem_reqs(&(vmm.vaddr_space_mm), kBytesin4GB);
    size_t pmmStorageSize = pmm_mem_reqs(&pmm, kBytesin4GB);

    uint8_t* vmmStorage    = memheap;
    memheap += vmmStorageSize;
    _end    += vmmStorageSize;
    uint8_t* pmmStorage    = memheap;
    memheap += pmmStorageSize;
    _end    += pmmStorageSize;

    pmm_init(&(vmm.vaddr_space_mm), 0x0, vmmStorage);
    pmm_init(&pmm, 0x0, pmmStorage);
    vmm_set_phys_mm(&pmm);
    vmm_set_current(&vmm);
    
    bool failed = false;
    memset(vmm.page_table.master, 0, sizeof(pde_t)*MMU_MASTER_ENTRY_COUNT);
    vaddr_t vaddr = vmm_alloc_n(kPagesIn4GB, &failed); // allocate
    if (failed) kprintf("vmm_alloc_n failed!\n");

    // REMOVE: debug
    // page_table_ddump(&(vmm.page_table), 0, 1);
    
    // REMOVE: coarse table at first 1 MB of master
    //init_table(l2, 0, 0, 0x100000);
    //master[0] = ((uint)l2 & 0xFFFFFC00);
    //master[0] |= 0x1;

    //uint tlb_l1_base = (uint)&master;
    // asm("mcr p15, 0, %0, c2, c0, 0" : : "r" (master) : "memory");

    // domain AP: "manager" (AP not checked)
    asm("mov r0, #0x3");
    asm("mcr p15, 0, r0, c3, c0, 0");

    // enable
    asm("mrc p15, 0, r0, c1, c0, 0");
    asm("orr r0, r0, #0x1");
    asm("mcr p15, 0, r0, c1, c0, 0");
    kprintf("MMU enabled.\r\n");
    
    return OK;
}
