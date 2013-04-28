/**
 * ARMv6 4k page Virtual Memory Manager
 *
 * Date: April 6, 2013
 * Author(s): Kathleen Banawa, Jedd Haberstro
 */

#include <stdint.h>

#define MMU_SMALL_PAGE_SIZE             4096
#define MMU_MASTER_ENTRY_COUNT          4096
#define MMU_COARSE_ENTRY_COUNT          256
#define MMU_MASTER_ALIGNMENT            0x4000

#define MMU_MASTER_ENTRY_ADDR_MASK      0xFFFFFC00
#define MMU_MASTER_ENTRY_ID             0x1

#define MMU_SMALL_ENTRY_XN              1
#define MMU_SMALL_ENTRY_BUFFERABLE      2
#define MMU_SMALL_ENTRY_CACHEABLE       3
#define MMU_SMALL_ENTRY_ACCESS_PERM     4
#define MMU_SMALL_ENTRY_BASE_ADDR       5

#define MMU_ENTRY_BASE_ADDR             1
#define MMU_ENABLED                     1
#define MMU_DISABLED                    0
#define MMU_PERMISSIONS_KRW_URW         1
#define MMU_PERMISSIONS_KRW_URW         1


typedef uintptr_t vaddr_t;
typedef uintptr_t paddr_t;
typedef uintptr_t pde_t;
typedef int small_entry_opt_t;
typedef int coarse_entry_opt_t;
typedef int small_entry_val_t;
typedef int coarse_entry_val_t;

typedef struct __attribute__ ((aligned(MMU_MASTER_ALIGNMENT))){
    pde_t table_entries[MMU_MASTER_ENTRY_COUNT];
} master_t;

typedef struct {
    pde_t small_entries[MMU_COARSE_ENTRY_COUNT];
} coarse_table_t;

void pde_coarse_set(coarse_entry_opt_t option, coarse_entry_val_t value);
void pde_small_set(small_entry_opt_t option, small_entry_val_t value);
pde_t pde_coarse_get(coarse_entry_opt_t option);
pde_t pde_small_get(small_entry_opt_t option);

// Maps 1 (4 kb) page
void map(coarse_table_t* tables, paddr_t phys, vaddr_t virt);
