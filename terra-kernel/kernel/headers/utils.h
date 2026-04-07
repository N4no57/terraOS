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
#define KERNEL_END  0xFFFFFFFF80200000
#define PAGE_SIZE   0x1000

#define NULL ((void*)0)

#define IA32_EFER 0xC0000080
#define STAR_MSR 0xC0000081
#define LSTAR_MSR 0xC0000082
#define CSTAR_MSR 0xC0000083
#define SFMASK_MSR 0xC0000084

typedef struct {
    u64 base_addr;
    u64 length;
    u32 type;
    u32 acpi_ext; // optional (if ECX >= 24)
} bios_mmap_entry;

struct regs {
    u64 rax;
    u64 rbx;
    u64 rcx;
    u64 rdx;
    u64 rsi;
    u64 rdi;
    u64 rsp;
    u64 rbp;
    u64 r8;
    u64 r9;
    u64 r10;
    u64 r11;
    u64 r12;
    u64 r13;
    u64 r14;
    u64 r15;
};

struct task {
    u32 pid;
    u64 *pml4t;
    struct regs regs;
    u8 state;
};

void* alloc_page();
void* memcpy(void* dest, const void* src, size_t count);
void *memset(void* dest, int c, size_t count);
void panic(const char* message) __attribute__((noreturn));

u64 read_msr(u32 msr);
void write_msr(u32 msr, u64 val);

extern u16 *VGA_MEMORY;

extern u64 next_free;
extern u64 *pml4t; // kernel page table
extern u8 *mem_bitmap;
extern u64 mem_bitmap_size; // in bytes
extern u64 mem_bitmap_bit_size;
extern u64 temp_page; // place where temporary page mappings go

#endif