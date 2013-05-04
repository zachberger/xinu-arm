#ifndef INCLUDE_VMM_H
#define INCLUDE_VMM_H

#ifdef PLATFORM_RASPBERRY_PI
#   include <mmu.h>
#else
#   error "Undefined platform."
#endif

/**
 * A vmm_t structure represents a virtual address space and the page table that maps that virtual address space.
 */
typedef struct vmm_t {
    page_table_t page_table;
    pmm_t vaddr_space_mm;
} vmm_t;

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

#endif // INCLUDE_VMM_H
