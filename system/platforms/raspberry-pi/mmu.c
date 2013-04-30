#include "mmu.h"
#include <stddef.h>

static vmm_t* _current_page_table;
static pmm_t* _phys_mm;

static inline void _vaddr_to_table_page_nums(vaddr_t virt, size_t* table_num, size_t* page_num) {
    *table_num = virt / 0x100000;
    *page_num = (virt & 0xFF000) >> 12;
}

void page_table_init(page_table_t* page_table) {
	int i;
	for (i = 0; i < MMU_MASTER_ENTRY_COUNT; ++i) {
		pde_t entry = ((uint32_t)&(page_table->l2[i])) & MMU_MASTER_ENTRY_ADDR_MASK;
		entry |= 0x1;
		page_table->master[i] = entry;
	}
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

void map(page_table_t* table, paddr_t phys, vaddr_t virt) {
	size_t table_num, page_num;
	// master_index = phys % MMU_COARSE_TABLE_MAPPING_SIZE;
	_vaddr_to_table_page_nums(virt, &table_num, &page_num);

    size_t entry = phys & 0xFFFFF000;
    entry |= 0x3;
    entry |= 0x30;
    
    (*(table->l2 + table_num)).small_entries[page_num] = entry;
}

paddr_t unmap(page_table_t* table, vaddr_t virt) {
// size_t table_num, page_num;
// 	_vaddr_to_table_page_nums(virt, &table_num, &page_num);
// 	
// 	size_t entry = (*_get_coarse_table(tables, table_num)).small_entries[page_num];
// 	paddr_t phys_addr = entry & 0xFFFFF000;
// 	entry = 0; // TODO - validate that just setting to 0 is correct
// 			   //	   - does this need a TLB flush also?
//     (*(table->l2 + table_num)).small_entries[page_num] = entry;
// 	
// 	return phys_addr;
	
	// TODO !
	return 0;
}

void vmm_set_current(vmm_t* new_vmm) {
    // TODO flush tlb, or use the process identifiers
    _current_page_table = new_vmm;
    asm("mcr p15, 0, %0, c2, c0, 0" : : "r" (_current_page_table->page_table.master) : "memory");
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
			map(&(_current_page_table->page_table), phys_addr, curr_vaddr);
			curr_vaddr += PMM_FRAME_SIZE;
		}
	}
	
	return vaddr;
}

void vmm_free(vaddr_t vaddr, size_t num_pages) {
	int i;
	for (i = 0; i < num_pages; ++i, vaddr += PMM_FRAME_SIZE) {
		paddr_t phys_addr = unmap(&(_current_page_table->page_table), vaddr);
		pmm_free_frame(&(_current_page_table->vaddr_space_mm), vaddr);
		pmm_free_frame(_phys_mm, phys_addr);
	}
}

