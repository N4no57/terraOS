#include "headers/pmm.h"

void pmm_init(bios_mmap_entry *mmap, u64 mmap_count) {
    u64 last_usable_end = 0;
    u64 usable_ram = 0;

    for (u64 i = 0; i < mmap_count; i++) {
        bios_mmap_entry *entry = &mmap[i];
        if (entry->type == 1) {
            u64 end = entry->base_addr + entry->length;
            usable_ram += entry->length;
            if (end > last_usable_end) last_usable_end = end;
        }
    }

    u64 num_pages = (last_usable_end + PAGE_SIZE - 1) / PAGE_SIZE;
    mem_bitmap_size = (num_pages + 7) / 8;
    u64 page_count = (mem_bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE; // the time has come to allocate the bitmap

    mem_bitmap = alloc_page();

    for (u64 i = 0; i < page_count - 1; i++) {
        alloc_page();
    }

    memset(mem_bitmap, 0xFF, mem_bitmap_size);

    // mark usable memory as free in 4KiB chunks
    for (u64 i = 0; i < mmap_count; i++) {
        bios_mmap_entry *entry = &mmap[i];
        if (entry->type != 1) continue;
        if (entry->base_addr > last_usable_end) continue;
        for (u64 i = (entry->base_addr + PAGE_SIZE - 1) / PAGE_SIZE; i < (entry->base_addr + entry->length + PAGE_SIZE - 1) / PAGE_SIZE; i++) {
            u64 array_index = i / 8;
            u8 bit_offset = i % 8;
            mem_bitmap[array_index] &= ~(1 << bit_offset);
        }
    }

    // tell the pmm that the space allocated by the page table to the kernel is in fact not usable
    for (u64 i = 0; i < (POOL_END - KERNEL_BASE + PAGE_SIZE - 1) / PAGE_SIZE; i++) {
        u64 array_index = i / 8;
        u8 bit_offset = i % 8;
        mem_bitmap[array_index] |= (1 << bit_offset);
    }
}