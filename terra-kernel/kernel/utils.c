#include "headers/utils.h"

void* alloc_page() {
    if (next_free + PAGE_SIZE >= POOL_END) {
        // panic: out of memory in bootstrap pool
        return NULL;
    }
    void* vaddr = (void*)next_free;
    next_free += PAGE_SIZE;
    return vaddr;
}

void* memcpy(void* dest, const void* src, size_t count) {
    char* d = dest;
    const char* s = src;
    for (int i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

void *memset(void* dest, int c, size_t count) {
    u8 *ptr = (u8*) dest;
    u8 byte_val = (u8) c;
    for (size_t i = 0; i < count; i++) {
        ptr[i] = byte_val;
    }
    return dest;
}

void panic(const char* message) {
    i32 y = 12;
    for (i32 i = 0; message[i] != '\0'; i++) {
        VGA_MEMORY[i + y * 80] = 0x0F00 | message[i]; // white text on black background
    }
    while (1) {
        __asm__ volatile ("hlt");
    }
}

u64 read_msr(u32 msr) {
    u32 low, high;

    __asm__ volatile (
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );

    return ((u64)high << 32) | low;
}

void write_msr(u32 msr, u64 val) {
    u32 low = (u32)(val & 0xFFFFFFFF);
    u32 high = (u32)(val >> 32);

    __asm__ volatile (
        "wrmsr"
        :
        : "c"(msr), "a"(low), "d"(high)
    );
}

typedef struct __attribute__((packed)) {
    u16 limit;
    u64 base;
} gdt_descriptor_t;

typedef struct __attribute__((packed)) {
    u16 limit_low;
    u16 base_low;
    u8 base_lmid;
    u8 access_byte;
    u8 flags_limit_high;
    u8 base_hmid;
    u32 base_high;
    u32 reserved;
} gdt_entry_64;

typedef struct __attribute__((packed)) {
    u32 reserved1;
    u64 rsp0;
    u64 rsp1;
    u64 rsp2;
    u64 reserved2;
    u64 ist1;
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u64 reserved3;
    u16 reserved4;
    u16 io_map_base;
} tss_t;

void finish_tss_setup() {
    gdt_descriptor_t gdtr;

    asm volatile (
        "sgdt %0"
        : "=m"(gdtr)
    );

    gdt_entry_64 *tss_descriptor;

    __asm__ volatile (
        "mov %1, %%rsi\n\t"
        "add $0x30, %%rsi\n\t"
        "mov %%rsi, %0\n\t"
        : "=m"(tss_descriptor)
        : "g"(gdtr.base)
        : "%rsi"
    );

    u64 base = tss_descriptor->base_low | ((u64)tss_descriptor->base_lmid << 16) | ((u64)tss_descriptor->base_hmid << 24) | ((u64)tss_descriptor->base_high << 32);
    tss_t *tss = (tss_t *)(base);

    tss->rsp0 = ((u64)alloc_page() + PAGE_SIZE) & -16; // allocate a new stack for ring 0
    tss->ist1 = ((u64)alloc_page() + PAGE_SIZE) & -16; // allocate a new stack for IST1
}

void relocate_gdt() {
    gdt_descriptor_t gdtr;

    asm volatile (
        "sgdt %0"
        : "=m"(gdtr)
    );

    // Convert the physical/identity-mapped address
    // into its higher-half virtual address.
    gdtr.base += KERNEL_BASE;

    asm volatile (
        "lgdt %0"
        :
        : "m"(gdtr)
        : "memory"
    );
}