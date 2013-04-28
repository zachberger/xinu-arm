#include "mmu.h"
#include <stddef.h>

static vmm_t* _current_master;
static pmm_t _phys_mm;

static inline void _vaddr_to_table_page_nums(vaddr_t virt, size_t* table_num, size_t* page_num) {
    *table_num = virt / 0x100000;
    *page_num = (virt & 0xFF000) >> 12;
}

static inline coarse_table_t* _get_coarse_table(master_t* tables, size_t table_num) {
	return (coarse_table_t*)(tables->table_entries[table_num] & 0xFFFFFC00);
}

void map(master_t* tables, paddr_t phys, vaddr_t virt) {
	size_t table_num, page_num;
	_vaddr_to_table_page_nums(virt, &table_num, &page_num);

    size_t entry = phys & 0xfffff000;
    entry |= 0x3;
    entry |= 0x30;
    
    (*_get_coarse_table(tables, table_num)).small_entries[page_num] = entry;
}

paddr_t unmap(master_t* tables, vaddr_t virt) {
	size_t table_num, page_num;
	_vaddr_to_table_page_nums(virt, &table_num, &page_num);
	
	size_t entry = (*_get_coarse_table(tables, table_num)).small_entries[page_num];
	paddr_t phys_addr = entry & 0xfffff000;
	entry = 0; // TODO - validate that just setting to 0 is correct
			   //	   - does this need a TLB flush also?
    (*_get_coarse_table(tables, table_num)).small_entries[page_num] = entry;
	
	return phys_addr;
}

void vmm_set_current(vmm_t* new_vmm) {
    // TODO flush tlb, or use the process identifiers
    _current_master = new_vmm;
    asm("mcr p15, 0, %0, c2, c0, 0" : : "r" (&(_current_master->master_table)) : "memory");
}

vmm_t* vmm_get_current(void) {
	return _current_master;
}

vaddr_t vmm_alloc(void) {
    return vmm_alloc_n(1);
}

vaddr_t vmm_alloc_n(size_t num_pages) {	
    // alloc virtual address space
	vaddr_t vaddr = pmm_alloc_frames(&(_current_master->vaddr_space_mm), num_pages);
	
	if (vaddr != NULL) {
		vaddr_t curr_vaddr = vaddr;
		int i;
		for (i = 0; i < num_pages; ++i) {
			// allocate a physical frame of memory
			paddr_t phys_addr = pmm_alloc_frame(&_phys_mm);
			
			// ran out of physical memory, so free any physical and virtual memory we've all ready allocated for this vmm_alloc.
			if (phys_addr == NULL) {
				vmm_free(vaddr, i);
				return NULL;
			}
			
			// map the physical frame to the virtual address
			map((master_t*)&(_current_master->master_table), phys_addr, curr_vaddr);
			curr_vaddr += PMM_FRAME_SIZE;
		}
	}
	
	return vaddr;
}

void vmm_free(vaddr_t vaddr, size_t num_pages) {
	int i;
	for (i = 0; i < num_pages; ++i, vaddr += PMM_FRAME_SIZE) {
		paddr_t phys_addr = unmap(&(_current_master->master_table), vaddr);
		pmm_free_frame(&(_current_master->vaddr_space_mm), vaddr);
		pmm_free_frame(&_phys_mm, phys_addr);
	}
}

