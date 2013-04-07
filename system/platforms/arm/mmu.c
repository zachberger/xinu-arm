#include "mmu.h"

void map(coarse_table_t* tables, paddr_t phys, vaddr_t virt) {
    size_t table_num = virt / 0x100000;
    size_t page_num = (virt & 0xFF000) >> 12;

    size_t entry = phys & 0xfffff000;
    entry |= 0x3;
    entry |= 0x30;
    
    (*(tables + table_num)).small_entries[page_num] = entry;
}
