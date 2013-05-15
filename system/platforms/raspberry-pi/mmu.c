#include "mmu.h"
#include "pmm.h"
#include "vmm.h"
#include <stddef.h>

static vmm_t* _current_page_table;
static pmm_t* _phys_mm;

void pde_coarse_set(pde_t *entry, coarse_entry_opt_t option, coarse_entry_val_t value) {
    int shift = 0;
    if (option == MMU_COARSE_ENTRY_BASE_ADDR) {
        *entry |= value & MMU_MASTER_ENTRY_ADDR_MASK;
    } else {
        switch (option) {
            case MMU_COARSE_ENTRY_ID: break;
            default:                  return;
        }
        *entry |= value << shift;
    }
}

void pde_small_set(pde_t *entry, small_entry_opt_t option, small_entry_val_t value) {
    int shift = 0;
    if (option == MMU_SMALL_ENTRY_BASE_ADDR) {
        *entry |= value & MMU_SMALL_ENTRY_ADDR_MASK;
    } else {
        switch (option) {
            case MMU_SMALL_ENTRY_XN:          break;
            case MMU_SMALL_ENTRY_ID:          shift = 1; break;
            case MMU_SMALL_ENTRY_BUFFERABLE:  shift = 2; break;
            case MMU_SMALL_ENTRY_CACHEABLE:   shift = 3; break;
            case MMU_SMALL_ENTRY_ACCESS_PERM: shift = 4; break;
            default:                          return;
        }
        *entry |= value << shift;
    }
}

coarse_entry_val_t pde_coarse_get(pde_t *entry, coarse_entry_opt_t option) {
    int start = 0, offset = 0;
    switch (option) {
        case MMU_COARSE_ENTRY_BASE_ADDR: start = 10; offset = 22; break;
        default:                         return *entry;
    }
    return (*entry >> start) & ((1 << offset) - 1);
}

small_entry_val_t pde_small_get(pde_t *entry, small_entry_opt_t option) {
    int start = 0, offset = 0;
    switch (option) {
        case MMU_SMALL_ENTRY_XN:          start = 0; break;
        case MMU_SMALL_ENTRY_BUFFERABLE:  start = 2; break;
        case MMU_SMALL_ENTRY_CACHEABLE:   start = 3; break;
        case MMU_SMALL_ENTRY_ACCESS_PERM: start = 4; offset = 2; break;
        case MMU_SMALL_ENTRY_BASE_ADDR:   start = 12; offset = 20; break;
        default:                          return *entry;
    }
    return (*entry >> start) & ((1 << offset) - 1);
}

static inline void _vaddr_to_table_page_nums(vaddr_t virt, size_t* table_num, size_t* page_num) {
    *table_num = virt / 0x100000;
    *page_num = (virt & 0xFF000) >> 12;
}

void page_table_ddump(page_table_t* page_table, size_t first_coarse, size_t num_coarses) {
    kprintf("=====================\r\npage_table_t debug dump start\r\n=====================\r\n");
    int i;
    for (i = first_coarse; i < first_coarse + num_coarses; ++i) {
        kprintf("Dumping coarse table %0d\r\n", i);
        coarse_table_t* coarse = &(page_table->l2[i]);
        int j;
        for (j = 0; j < MMU_COARSE_ENTRY_COUNT; ++j) {
            kprintf("\tsmall entry %0d : %x\r\n", j, coarse->small_entries[j]);
        }
    }
    kprintf("=====================\r\npage_table_t debug dump end\r\n=====================\r\n");
}

void page_table_map(page_table_t* table, paddr_t phys, vaddr_t virt) {
    size_t table_num, page_num;
    size_t master_index = phys / MMU_COARSE_TABLE_MAPPING_SIZE;
    
    if (table->master[master_index] == 0) {
        pde_t entry = 0;
        pde_coarse_set(&entry, MMU_COARSE_ENTRY_BASE_ADDR, (uint32_t)&(table->l2[master_index]));
        pde_coarse_set(&entry, MMU_COARSE_ENTRY_ID, 1);
        table->master[master_index] = entry;
    }
    
    _vaddr_to_table_page_nums(virt, &table_num, &page_num);
    pde_t entry = 0;
    pde_small_set(&entry, MMU_SMALL_ENTRY_ID, 1);
    pde_small_set(&entry, MMU_SMALL_ENTRY_ACCESS_PERM, MMU_PERMISSIONS_RWRW);
    pde_small_set(&entry, MMU_SMALL_ENTRY_BASE_ADDR, phys);
    (*(table->l2 + table_num)).small_entries[page_num] = entry;
}

paddr_t page_table_unmap(page_table_t* table, vaddr_t virt) {
    int i;
    size_t master_index; 
    bool all_zero = true;
    coarse_table_t *coarse = NULL;
    size_t table_num, page_num;
    
    coarse = table->l2 + table_num;
    _vaddr_to_table_page_nums(virt, &table_num, &page_num);
    
    size_t entry = coarse->small_entries[page_num];
    paddr_t phys_addr = entry & MMU_SMALL_ENTRY_ADDR_MASK;
    // TODO - validate that just setting to 0 is correct
    //      - does this need a TLB flush also?
    coarse->small_entries[page_num] = entry;

    for (i = 0; i < MMU_COARSE_ENTRY_COUNT; ++i) {
        if (coarse->small_entries[i] != 0) {
            all_zero = false;
            break;
        }
    }

    if (all_zero) {
        master_index = phys_addr / MMU_COARSE_TABLE_MAPPING_SIZE;
        table->master[master_index] = 0;
    }
    
    return phys_addr;
}

void vmm_set_current(vmm_t* new_vmm) {
    // TODO flush tlb, or use the process identifiers
    _current_page_table = new_vmm;
    asm("mcr p15, 0, %0, c2, c0, 0" : : "r" (_current_page_table->page_table->master) : "memory");
}

vmm_t* vmm_get_current(void) {
    return _current_page_table;
}

void vmm_set_phys_mm(pmm_t* phys_mm) {
    _phys_mm = phys_mm;
}

vaddr_t vmm_alloc(bool* out_failed) {
    return vmm_alloc_n(1, out_failed);
}

vaddr_t vmm_alloc_n(size_t num_pages, bool* out_failed) {   
    // alloc virtual address space
    bool failed = false;
    vaddr_t vaddr = pmm_alloc_frames(&(_current_page_table->vaddr_space_mm), num_pages, &failed);
    if (out_failed) *out_failed = failed;
    
    if (failed == false) {
        vaddr_t curr_vaddr = vaddr;
        int i;
        kprintf("mapping %d pages [vmm_alloc_n]\r\n", num_pages);
        for (i = 0; i < num_pages; ++i) {
            // allocate a physical frame of memory
            paddr_t phys_addr = pmm_alloc_frame(_phys_mm, &failed);
            
            // ran out of physical memory, so free any physical and virtual memory we've all ready allocated for this vmm_alloc.
            if (failed == true) {
                vmm_free(vaddr, i);
                if (out_failed) *out_failed = true;
                return NULL;
            }
            
            // map the physical frame to the virtual address
            page_table_map(&(_current_page_table->page_table), phys_addr, curr_vaddr);
            curr_vaddr += PMM_FRAME_SIZE;
        }
    }
    
    return vaddr;
}

void vmm_free(vaddr_t vaddr, size_t num_pages) {
    int i;
    for (i = 0; i < num_pages; ++i, vaddr += PMM_FRAME_SIZE) {
        paddr_t phys_addr = page_table_unmap(&(_current_page_table->page_table), vaddr);
        pmm_free_frame(&(_current_page_table->vaddr_space_mm), vaddr);
        pmm_free_frame(_phys_mm, phys_addr);
    }
}

