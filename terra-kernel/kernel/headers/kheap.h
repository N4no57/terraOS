#ifndef KHEAP_H
#define KHEAP_H

#include "utils.h"

extern void *heap_start;
extern void *heap_end;

void heap_init(void);
void* kalloc(size_t size);
void* krealloc(void *ptr, size_t new_size);
void kfree(void* ptr);

#endif
