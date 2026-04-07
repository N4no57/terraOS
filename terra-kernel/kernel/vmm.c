#include "headers/vmm.h"
#include "headers/pmm.h"

#define PAGE_MASK 0x000FFFFFFFFFF000
#define TEMP_PAGE 0xFFFFFFFF801FF000

/**
 * @param addr A virtual address
 */
static inline void tlb_invalidate(void *addr) {
    asm volatile("invlpg (%0)" :: "r"(addr) : "memory");
}

void temp_map(u64 physical_address) {
    u64 pml4t_index = (TEMP_PAGE >> 39) & 0x1FF;
    u64 pdpt_index = (TEMP_PAGE >> 30) & 0x1FF;
    u64 pd_index = (TEMP_PAGE >> 21) & 0x1FF;
    u64 pt_index = (TEMP_PAGE >> 12) & 0x1FF;

    u64 *pdpt = (u64 *)((pml4t[pml4t_index] & PAGE_MASK) + KERNEL_BASE);
    u64 *pd = (u64 *)((pdpt[pdpt_index] & PAGE_MASK) + KERNEL_BASE);
    u64 *pt = (u64 *)((pd[pd_index] & PAGE_MASK) + KERNEL_BASE);
    pt[pt_index] = physical_address | 0x13;
    tlb_invalidate((void *)TEMP_PAGE);
}

/**
 * @param ctx a pointer to the page table to be modified with some metadata
 * @param virtual_address address to which you want to map a physical address to
 * @param physical_address actual memory address
 * @param flags some bits to tell the MMU what it should think of this page table entry
 */
void map_page(page_table_t ctx, u64 virtual_address, u64 physical_address, u64 flags) {
    u64 pml4t_index = (virtual_address >> 39) & 0x1FF;
    u64 pdpt_index = (virtual_address >> 30) & 0x1FF;
    u64 pd_index = (virtual_address >> 21) & 0x1FF;
    u64 pt_index = (virtual_address >> 12) & 0x1FF;

    u64 entry = ctx.pml4t[pml4t_index];
    u64 *pdpt = (u64 *)TEMP_PAGE;
    if (((u64)entry & 0x1) == 0) { // check if present bit exists
        void *check = pmm_alloc();
        if (!check) {
            panic("Out of physical memory for page table!");
        }
        entry = (u64)check;
        ctx.pml4t[pml4t_index] = entry | flags | 0x1;
        temp_map(entry);
        memset((void *)TEMP_PAGE, 0, 0x1000);
    } else {
        temp_map((entry & PAGE_MASK));
    }

    entry = pdpt[pdpt_index];
    u64 *pd = (u64 *)TEMP_PAGE;
    if (((u64)entry & 0x1) == 0) {
        void *check = pmm_alloc();
        if (!check) {
            panic("Out of physical memory for page table!");
        }
        entry = (u64)check;
        pdpt[pdpt_index] = entry | flags | 0x1;
        temp_map(entry);
        memset((void *)TEMP_PAGE, 0, 0x1000);
    } else {
        temp_map((entry & PAGE_MASK));
    }

    entry = pd[pd_index];
    u64 *pt = (u64 *)TEMP_PAGE;
    if (((u64)entry & 0x1) == 0) {
        void *check = pmm_alloc();
        if (!check) {
            panic("Out of physical memory for page table!");
        }
        entry = (u64)check;
        pd[pd_index] = entry | flags | 0x1;
        temp_map(entry);
        memset((void *)TEMP_PAGE, 0, 0x1000);
    } else {
        temp_map((entry & PAGE_MASK));
    }

    pt[pt_index] = physical_address | flags;
}

/**
 * @param ctx basically a pointer to a page table in a nice wrapper
 * @param virtual_address the virtual address the you want to be unmapped
 */
void unmap_page(page_table_t ctx, u64 virtual_address) {
    u64 pml4t_index = (virtual_address >> 39) & 0x1FF;
    u64 pdpt_index = (virtual_address >> 30) & 0x1FF;
    u64 pd_index = (virtual_address >> 21) & 0x1FF;
    u64 pt_index = (virtual_address >> 12) & 0x1FF;

    void *pdpt_phys = (void *)(ctx.pml4t[pml4t_index] & PAGE_MASK);
    u64 *pdpt = (u64 *)TEMP_PAGE;
    temp_map((u64)pdpt_phys);
    
    void *pd_phys = (void *)(pdpt[pdpt_index] & PAGE_MASK);
    u64 *pd = (u64 *)TEMP_PAGE;
    temp_map((u64)pd_phys);

    void *pt_phys = (void *)(pd[pd_index] & PAGE_MASK);
    u64 *pt = (u64 *)TEMP_PAGE;
    temp_map((u64)pt_phys);

    void *pte = (void *)(pt[pt_index] & PAGE_MASK);

    pt[pt_index] = 0;

    pmm_free(pte);

    temp_map((u64)pt_phys);
    bool empty = true;
    for (u16 i = 0; i < 512; i++) {
        if (pt[i] & 0x1) {
            empty = false;
            break;
        }
    }

    if (empty) {
        pmm_free(pt_phys);
    }

    temp_map((u64)pd_phys);
    empty = true;
    for (u16 i = 0; i < 512; i++) {
        if (pd[i] & 0x1) {
            empty = false;
            break;
        }
    }

    if (empty) {
        pmm_free(pd_phys);
    }

    temp_map((u64)pdpt_phys);
    empty = true;
    for (u16 i = 0; i < 512; i++) {
        if (pdpt[i] & 0x1) {
            empty = false;
            break;
        }
    }

    if (empty) {
        pmm_free(pdpt_phys);
    }
}

/**
 * @param ctx basically a pointer to a page table in a nice wrapper
 * @param virtual_address the virtual address the you want to be unmapped
 * @return the physical address that the virtual address in this page table is allocated to
 */
u64 get_physical_address(page_table_t ctx, u64 virtual_address) {
    u64 pml4t_index = (virtual_address >> 39) & 0x1FF;
    u64 pdpt_index = (virtual_address >> 30) & 0x1FF;
    u64 pd_index = (virtual_address >> 21) & 0x1FF;
    u64 pt_index = (virtual_address >> 12) & 0x1FF;

    void *pdpt_phys = (void *)(ctx.pml4t[pml4t_index] & PAGE_MASK);
    u64 *pdpt = (u64 *)TEMP_PAGE;
    temp_map((u64)pdpt_phys);
    
    void *pd_phys = (void *)(pdpt[pdpt_index] & PAGE_MASK);
    u64 *pd = (u64 *)TEMP_PAGE;
    temp_map((u64)pd_phys);

    void *pt_phys = (void *)(pd[pd_index] & PAGE_MASK);
    u64 *pt = (u64 *)TEMP_PAGE;
    temp_map((u64)pt_phys);

    return (pt[pt_index] & PAGE_MASK) + (virtual_address & 0xFFF);
}
