#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef u64 size_t;

#define KERNEL_BASE 0xFFFFFFFF80000000
#define POOL_START  0xFFFFFFFF80020000
#define POOL_END    0xFFFFFFFF801FF000
#define PAGE_SIZE   0x1000

#define NULL ((void*)0)

typedef struct {
    u64 base_addr;
    u64 length;
    u32 type;
    u32 acpi_ext; // optional (if ECX >= 24)
} bios_mmap_entry;

void* alloc_page();
void* memcpy(void* dest, const void* src, size_t count);
void *memset(void* dest, int c, size_t count);
void panic(const char* message) __attribute__((noreturn));

extern u16 *VGA_MEMORY;

extern u64 next_free;
extern u64 *pml4t; // kernel page table
extern u8 *mem_bitmap;
extern u64 mem_bitmap_size; // in bytes
extern u64 mem_bitmap_bit_size;
extern u64 temp_page; // place where temporary page mappings go

#endif