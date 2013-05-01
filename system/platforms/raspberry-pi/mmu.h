/**
 * ARMv6 4k page Virtual Memory Manager
 *
 * Date: April 6, 2013
 * Author(s): Kathleen Banawa, Jedd Haberstro
 */

#ifndef SYSTEM_PLATFORMS_RASPBERRYPI_MMU_H
#define SYSTEM_PLATFORMS_RASPBERRYPI_MMU_H

#include "pmm.h"
#include <stdint.h>
#include <stdbool.h>

#define MMU_SMALL_PAGE_SIZE             4096
#define MMU_MASTER_ENTRY_COUNT          4096
#define MMU_COARSE_ENTRY_COUNT          256
#define MMU_MASTER_ALIGNMENT            0x4000
#define MMU_COARSE_TABLE_MAPPING_SIZE   (MMU_COARSE_ENTRY_COUNT * PMM_FRAME_SIZE) // 4096 mb

#define MMU_MASTER_ENTRY_ADDR_MASK      0xFFFFFC00
#define MMU_SMALL_ENTRY_ADDR_MASK       0xFFFFF000

#define MMU_SMALL_ENTRY_XN              1
#define MMU_SMALL_ENTRY_BUFFERABLE      2
#define MMU_SMALL_ENTRY_CACHEABLE       3
#define MMU_SMALL_ENTRY_ACCESS_PERM     4
#define MMU_SMALL_ENTRY_BASE_ADDR       5
#define MMU_SMALL_ENTRY_ID              6

#define MMU_COARSE_ENTRY_BASE_ADDR      1
#define MMU_COARSE_ENTRY_ID             2

#define MMU_PERMISSIONS_NANA            0
#define MMU_PERMISSIONS_RWNA            1
#define MMU_PERMISSIONS_RWRO            2
#define MMU_PERMISSIONS_RWRW            3

#define MMU_ENABLED                     1
#define MMU_DISABLED                    0


typedef uintptr_t vaddr_t;
typedef uintptr_t paddr_t;
typedef uintptr_t pde_t;
typedef int small_entry_opt_t;
typedef int coarse_entry_opt_t;
typedef int small_entry_val_t;
typedef int coarse_entry_val_t;

typedef struct coarse_table_t {
    pde_t small_entries[MMU_COARSE_ENTRY_COUNT];
} coarse_table_t;

typedef struct page_table_t {
    pde_t master[MMU_MASTER_ENTRY_COUNT] __attribute__ ((aligned(MMU_MASTER_ALIGNMENT)));
    coarse_table_t l2[MMU_MASTER_ENTRY_COUNT];
} page_table_t;

/**
 * A vmm_t structure represents a virtual address space and the page table that maps that virtual address space.
 */
typedef struct vmm_t {
    page_table_t page_table;
    pmm_t vaddr_space_mm;
} vmm_t;

/**
 * Modifies the given attribute of a coarse page descriptor entry
 *
 * entry  - a coarse page descriptor entry
 * option - the requested attribute
 * value  - the value of the attribute to set
 */
void pde_coarse_set(pde_t *entry, coarse_entry_opt_t option, coarse_entry_val_t value);

/**
 * Modifies the given attribute of a small page descriptor entry
 * 
 * entry  - a small page descriptor entry
 * option - the requested attribute
 * value  - the value of the attribute to set
 */
void pde_small_set(pde_t *entry, small_entry_opt_t option, small_entry_val_t value);

/**
 * Retrieves the given attribute of a coarse page descriptor entry
 *
 * entry   - a coarse page descriptor entry
 * option  - the requested attribute
 * returns - the value of the attribute
 */
coarse_entry_val_t pde_coarse_get(pde_t *entry, coarse_entry_opt_t option);

/**
 * Retrieves the given attribute of a small page descriptor entry
 *
 * entry   - a small page descriptor entry
 * option  - the requested attribute
 * returns - the value of the attribute
 */
small_entry_val_t pde_small_get(pde_t *entry, small_entry_opt_t option);

/**
 * Print the contents of a page table using kprintf.
 * 
 * page_table   - the page table dump
 * first_coarse - the starting coarse table at which we will start printing.
 * num_coarses  - the number of coarse tables to print.
 */
void page_table_ddump(page_table_t* page_table, size_t first_coarse, size_t num_coarses);

/**
 * Map between the given physical address and virtual address one page (4kb) of memory.
 *
 * table - the table to map into
 * phys  - the physical address to map
 * virt  - the virtual address to map
 */ 
void page_table_map(page_table_t* table, paddr_t phys, vaddr_t virt);

/**
 * Undo the current mapping for the given virtual address
 *
 * table   - the table in which the mapping exists and will be undone
 * virt    - the virtual address to unmap
 * returns - the physical address that was previously mapped to the given virtual address
 */
paddr_t page_table_unmap(page_table_t* table, vaddr_t virt);

/**
 * Sets the current virtual memory manager, updates the page table the MMU uses, and performs a tlb flush.
 * 
 * new_vmm - the new virtual memory manager to installs
 */
void vmm_set_current(vmm_t* new_vmm);

/**
 * Set the physical memory manager that can be queried by vmm_alloc* to retrieve free physical memory.
 *
 * phys_mm - the physical memory manager
 */
void vmm_set_phys_mm(pmm_t* phys_mm);

/**
 * Get the current virtual memory manager.
 *
 * returns - the current virtual memory manager.
 */
vmm_t* vmm_get_current(void);

/**
 * Allocates one page of virtual memory, using the current virtual memory manager.
 * Equivalent to vmm_alloc_(1, out_failed).
 */
vaddr_t vmm_alloc(bool* out_failed);

/**
 * Allocate num_pages of physical memory, and maps the physical memory to num_pages of contigious virtual memory, using the current virtual memory manager.
 *
 * num_pages  - the number of contigious virtual pages to allocate.
 * out_failed - if non-NULL, will be true upon a successful allocation, or false upon an unsuccessful allocation.
 * returns    - the virtual address to the first newly allocated virtual page.
 */ 
vaddr_t vmm_alloc_n(size_t num_pages, bool* out_failed);

/**
 * Free num_pages of contigious virtual memory starting at vaddr. Unmaps the virtual addresses in the current virtual memory manager and returns the physical memory to the physical memory manager.
 *
 * vaddr     - the start of virtual memory to free.
 * num_pages - the number of virtual pages to free.
 */
void vmm_free(vaddr_t vaddr, size_t num_pages);

#endif // SYSTEM_PLATFORMS_RASPBERRYPI_MMU_H


