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