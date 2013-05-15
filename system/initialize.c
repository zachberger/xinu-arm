/**
 * @file initialize.c
 * @provides nulluser, sysinit.
 * The system begins intializing after the C environment has been
 * established.  After intialization, the null thread remains always in
 * a ready (THRREADY) or running (THRCURR) state.
 *
 * $Id: initialize.c 2115 2009-11-03 02:31:19Z brylow $
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <kernel.h>
#include <backplane.h>
#include <clock.h>
#include <device.h>
#include <gpio.h>
#include <memory.h>
#include <bufpool.h>
#include <platform.h>
#include <thread.h>
#include <tlb.h>
#include <queue.h>
#include <semaphore.h>
#include <mailbox.h>
#include <network.h>
#include <nvram.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <safemem.h>

#include <uart.h> //DEBUG


/* Linker provides start and end of image */
extern void _start(void);       /* start of Xinu code                  */

/* Function prototypes */
extern void main(void);         /* main is the first thread created    */
extern void xdone(void);        /* system "shutdown" procedure         */
extern int platforminit(void);  /* determines platform settings        */
static int sysinit(void);       /* intializes system structures        */

/* Declarations of major kernel variables */
struct thrent thrtab[NTHREAD];  /* Thread table                   */
struct sement semtab[NSEM];     /* Semaphore table                */
qid_typ readylist;              /* List of READY threads          */
struct memblock memlist;        /* List of free memory blocks     */
struct bfpentry bfptab[NPOOL];  /* List of memory buffer pools    */

/* Active system status */
int thrcount;                   /* Number of live user threads         */
tid_typ thrcurrent;             /* Id of currently running thread      */

/* Params set by startup.S */
void *memheap;                  /* Bottom of heap (top of O/S stack)   */
ulong cpuid;                    /* Processor id                        */

struct platform platform;       /* Platform specific configuration     */

/* System physical memory manager */
static pmm_t pmm;

void programSwitch( void );

/**
 * Intializes the system and becomes the null thread.
 * This is where the system begins after the C environment has been 
 * established.  Interrupts are initially DISABLED, and must eventually 
 * be enabled explicitly.  This routine turns itself into the null thread 
 * after initialization.  Because the null thread must always remain ready 
 * to run, it cannot execute code that might cause it to be suspended, wait 
 * for a semaphore, or put to sleep, or exit.  In particular, it must not 
 * do I/O unless it uses kprintf for synchronous output.
 */
int nulluser(void)
{
    platforminit();

    kprintf(VERSION);
    kprintf("\r\r\n\n");

#ifdef DETAIL
    /* Output detected platform. */
    kprintf("Processor identification: 0x%08X\r\n", cpuid);
    kprintf("Detected platform as: %s, %s\r\r\n\n",
            platform.family, platform.name);
#endif

    sysinit();

    /* Output Xinu memory layout */
    kprintf("%10d bytes physical memory.\r\n",
            (ulong)platform.maxaddr & 0x7FFFFFFF);
#ifdef KSEG0_BASE
#ifdef DETAIL
    kprintf("           [0x%08X to 0x%08X]\r\n",
            (ulong)KSEG0_BASE, (ulong)(platform.maxaddr - 1));
#endif
    kprintf("%10d bytes reserved system area.\r\n",
            (ulong)_start - KSEG0_BASE);
#ifdef DETAIL
    kprintf("           [0x%08X to 0x%08X]\r\n",
            (ulong)KSEG0_BASE, (ulong)_start - 1);
#endif
#endif /* KSEG0_BASE */

    kprintf("%10d bytes Xinu code.\r\n", (ulong)&_end - (ulong)_start);
#ifdef DETAIL
    kprintf("           [0x%08X to 0x%08X]\r\n",
            (ulong)_start, (ulong)&_end - 1);
#endif

    kprintf("%10d bytes stack space.\r\n", (ulong)memheap - (ulong)&_end);
#ifdef DETAIL
    kprintf("           [0x%08X to 0x%08X]\r\n",
            (ulong)&_end, (ulong)memheap - 1);
#endif

    kprintf("%10d bytes heap space.\r\n",
            (ulong)platform.maxaddr - (ulong)memheap);
#ifdef DETAIL
    kprintf("           [0x%08X to 0x%08X]\r\r\n\n",
            (ulong)memheap, (ulong)platform.maxaddr - 1);
#endif
    kprintf("\r\n");

#ifdef CONSOLE
    open(CONSOLE, SERIAL0);
#endif /* CONSOLE */

    /* enable interrupts here */
    enable();

#if 0
    ready(create
          ((void *)main, INITSTK, INITPRIO, "MAIN", 2, 0,
           NULL), RESCHED_YES);
#else // Quick hack to fix bug in ctxsw for raspberry-pi: First resched *MUST* be called from an IRQ!!!
    ready(create
          ((void *)main, INITSTK, INITPRIO, "MAIN", 2, 0,
           NULL), RESCHED_NO);
#endif

    while (TRUE)
    {
//#ifndef DEBUG
        pause();
//#endif /* DEBUG */
    }
    return OK;
}

/**
 * Intializes all Xinu data structures and devices.
 * @return OK if everything is initialized successfully
 */
static int sysinit(void)
{
    int i;
    void *userheap;             /* pointer to user memory heap   */
    struct thrent *thrptr;      /* thread control block pointer  */
    device *devptr;             /* device entry pointer          */
    struct sement *semptr;      /* semaphore entry pointer       */
    struct memblock *pmblock;   /* memory block pointer          */
    struct bfpentry *bfpptr;

    /* Initialize system variables */
    /* Count this NULLTHREAD as the first thread in the system. */
    thrcount = 1;

    kprintf("variable i in this function is at 0x%x\r\n", &i);
    kprintf("this function sysinit is at 0x%x\r\n", &sysinit);
    kprintf("_start is at 0x%x\r\n", _start);
    kprintf("_end is at (the end) at 0x%x\r\n", &_end);
    kprintf("readylist is at 0x%x\r\n", &readylist);
    kprintf("kputc is at 0x%x\r\n", &kputc);
    kprintf("NTHREAD is %d\r\n", NTHREAD);

    kprintf("memheap is 0x%x\r\n", memheap);

    /* Initialize free memory list */
    memheap = roundmb(memheap);
    platform.maxaddr = truncmb(platform.maxaddr);
    // kprintf("platform.maxaddr is 0x%x\r\n", platform.maxaddr);
    // memlist.next = pmblock = (struct memblock *)memheap;
    // memlist.length = (uint)(platform.maxaddr - memheap);
    // pmblock->next = NULL;
    // pmblock->length = (uint)(platform.maxaddr - memheap);

    /* Initialize thread table */
    for (i = 0; i < NTHREAD; i++)
    {
        thrtab[i].state = THRFREE;
    }
    kprintf("thrtab is at 0x%x, size %d\r\n", thrtab, sizeof(thrtab));

    /* initialize null thread entry */
    thrptr = &thrtab[NULLTHREAD];
    thrptr->state = THRCURR;
    thrptr->prio = 0;
    strncpy(thrptr->name, "prnull", 7);
    thrptr->stkbase = (void *)&_end;
    thrptr->stklen = (ulong)memheap - (ulong)&_end;
    thrptr->stkptr = 0;
    thrcurrent = NULLTHREAD;
    
    kprintf("thrcount: %d\r\n", NTHREAD);

    kprintf("Starting MMU initialization\r\n");
    //size_t const kPagesIn4GB = 1048576;
    //uint64_t const kBytesin4GB = kPagesIn4GB * 4096ull;
    //size_t vmmStorageSize = pmm_mem_reqs(&(thrptr->vmm.vaddr_space_mm), kBytesin4GB);
    //size_t pmmStorageSize = pmm_mem_reqs(&pmm, kBytesin4GB/8); // TODO - instead of assuming 512 mb, query broadcom mailbox
    //uint8_t* vmmStorage    = memheap;
    //memheap += vmmStorageSize;
    //uint8_t* pmmStorage = memheap;
    //memheap += pmmStorageSize;

    //memset(thrptr->vmm.page_table.master, 0, sizeof(pde_t)*MMU_MASTER_ENTRY_COUNT);
    //kprintf("pmm_init\r\n");
    //pmm_init(&(thrptr->vmm.vaddr_space_mm), memheap, vmmStorage);
    //pmm_init(&pmm, 0x0, pmmStorage);
    //kprintf("vmm_set_phys_mm, vmm_set_current\r\n");
    //vmm_set_phys_mm(&pmm);
    //vmm_set_current(&(thrptr->vmm));

    // Kernel maps lower 1 gb, user maps starting at first mb
    #define KERNEL_BASE 0x0
    #define USER_BASE   0x40000000 
    
    // round _end to page size
    //memheap = PMM_ALIGN_ADDR(memheap);
    
    // map the kernel
    paddr_t cur_paddr;
    vaddr_t cur_vaddr;
    kprintf("Mapping\r\n");
    //for (cur_paddr = 0x0, cur_vaddr = 0x0; cur_paddr < (paddr_t)memheap/4096; cur_paddr += 1, cur_vaddr += 1) {
    //    page_table_map(&(thrptr->vmm.page_table), cur_paddr*PMM_FRAME_SIZE, cur_vaddr*PMM_FRAME_SIZE);
    //}
    //bool failed = false;
    //vmm_alloc_n((size_t)memheap/4096, &failed);
    //kprintf("%d\r\n", failed);
    
    kprintf("Turning on MMU.\r\n");
    
    // TODO - move mmu init code into proper function
#ifdef PLATFORM_RASPBERRY_PI
    //asm("mov r0, #0x3");
    //asm("mcr p15, 0, r0, c3, c0, 0");

    // enable
    //asm("mrc p15, 0, r0, c1, c0, 0");
    //asm("orr r0, r0, #0x1");
    //asm("mcr p15, 0, r0, c1, c0, 0");
    kprintf("MMU enabled.\r\n");
#else
#   warning "No MMU initialization code for current platform."
#endif
    kprintf("Ending MMU initialization.\r\n");
    

    kprintf("&_end is 0x%x\r\n", &_end);

    /* Initialize semaphores */
    for (i = 0; i < NSEM; i++)
    {
        semptr = &semtab[i];
        semptr->state = SFREE;
        semptr->count = 0;
        semptr->queue = queinit();
    }

    kprintf("NPOOL is %d\r\n", NPOOL);

    /* Initialize buffer pools */
    for (i = 0; i < NPOOL; i++)
    {
        bfpptr = &bfptab[i];
        bfpptr->state = BFPFREE;
    }
    kprintf("calling queinit() for readylist\r\n");

    /* initialize thread ready list */
    readylist = queinit();

#if SB_BUS
    backplaneInit(NULL);
#endif                          /* SB_BUS */

#if RTCLOCK
    /* initialize real time clock */
    kprintf("Clock being initialized.\r\n" );
    clkinit();
#endif                          /* RTCLOCK */

#ifdef UHEAP_SIZE //NOTE, false on PI
    /* Initialize user memory manager */
    userheap = stkget(UHEAP_SIZE);
    if (SYSERR != (int)userheap)
    {
        userheap = (void *)((uint)userheap - UHEAP_SIZE + sizeof(int));
        memRegionInit(userheap, UHEAP_SIZE);

        /* initialize memory protection */
        safeInit();
	
        /* initialize kernel page mappings */
        safeKmapInit();
    }
#else
    userheap = NULL;
#endif                          /* UHEAP_SIZE */

#if USE_TLB //NOTE, false on PI
    /* initialize TLB */
    tlbInit();
    /* register system call handler */
    exceptionVector[EXC_SYS] = syscall_entry;
#endif                          /* USE_TLB */

#if NMAILBOX //NOTE, true on PI
    /* intialize mailboxes */
    mailboxInit();
#endif

#if NDEVS //NOTE, nonzero on PI
    for (i = 0; i < NDEVS; i++)
    {
        if (!isbaddev(i))
        {
            devptr = (device *)&devtab[i];
            (devptr->init) (devptr);
        }
    }
#endif

#if 0
#if NVRAM
    nvramInit();
#endif
    kprintf("SO MUCH MORE NOT done with sysinit()\r\n");
#if NNETIF
    netInit();
#endif
    kprintf("NOT done with sysinit()\r\n");
#if GPIO
    gpioLEDOn(GPIO_LED_CISCOWHT);
#endif
#endif

    kprintf("done with sysinit()\r\n");

    return OK;
}

