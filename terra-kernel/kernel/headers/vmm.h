#ifndef VMM_H
#define VMM_H

#include "utils.h"
#include <stdbool.h>

#define TEMP_PAGE 0xFFFFFFFF801FF000

// page table flag bits:
// 0	P (Present)	Must be 1 or page fault occurs
// 1	RW (Read/Write)	0 = read-only, 1 = writable
// 2	US (User/Supervisor)	0 = kernel only, 1 = user accessible
// 3	PWT	Page-level write-through caching
// 4	PCD	Page-level cache disable
// 5	A (Accessed)	Set by CPU when accessed
// 6	D (Dirty)	Set on write (only meaningful in PTE)
// 7	PS (Page Size)	Only in PDE/PDPTE (huge pages)
// 8	G (Global)	Prevents TLB invalidation on CR3 reload
// 9–11	Available	Free for OS use

typedef struct {
    u64 *pml4t;
    bool is_kernel;
} page_table_t;

void map_page(page_table_t ctx, u64 virtual_address, u64 physical_address, u64 flags);
void unmap_page(page_table_t ctx, u64 virtual_address);
u64 get_physical_address(page_table_t ctx, u64 virtual_address);
void tlb_invalidate(void *addr);

#endif