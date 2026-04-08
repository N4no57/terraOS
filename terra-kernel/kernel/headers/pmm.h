#ifndef PMM_H
#define PMM_H

#include "utils.h"

void pmm_init(bios_mmap_entry *mmap, u64 mmap_count);
void* pmm_alloc();
void pmm_free(void* block);
void temp_map(u64 physical_address);

#endif