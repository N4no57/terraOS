#include "headers/idt.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef u64 size_t;

#define KERNEL_BASE 0xFFFFFFFF80000000
#define POOL_START  0xFFFFFFFF80020000
#define POOL_END    0xFFFFFFFF80200000
#define PAGE_SIZE   0x1000

#define NULL ((void*)0)

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t acpi_ext; // optional (if ECX >= 24)
} bios_mmap_entry;

void kernel_main(bios_mmap_entry *mmap, u64 mmap_count) __attribute__((section(".kernel")));
void panic(const char* message) __attribute__((noreturn));
void pmm_init(bios_mmap_entry *mmap, u64 mmap_count);
void sse_init();
void paging_init();
void* alloc_page();
u64 v2p(void* v);
void* memcpy(void* dest, const void* src, size_t count);
void *memset(void* dest, int c, size_t count);

u16 *VGA_MEMORY = (u16*)(KERNEL_BASE + 0xB8000);
u64 next_free = POOL_START;
u64 *pml4 = NULL;
u8 *mem_bitmap = NULL;
u64 mem_bitmap_size = 0; // in bytes

void kernel_main(bios_mmap_entry *mmap, u64 mmap_count) {
    // init IDT for reasons unbeknownst to man
    idt_init();
    sse_init();

    alloc_page(); // allocate the first page as this is where the stack lives, bootloader took care of setting it already

    u64 mmap_size = sizeof(bios_mmap_entry) * mmap_count;
    u64 page_count = (mmap_size + 4096 - 1) / 4096;
    for (u64 i = 0; i < page_count; i++) { // allocate memory for the mmap entries
        alloc_page();
    }

    paging_init();
    pmm_init(mmap, mmap_count);

    i32 y = 11;
    const char message[] = "TerraOS - 64-bit C Kernel\0";

    for (i32 i = 0; message[i] != '\0'; i++) {
        VGA_MEMORY[i + y * 80] = 0x0F00 | message[i]; // white text on black background
    }

    memset((void*)(KERNEL_BASE + 0xB8000), 0, 80 * 25 * 2); // clear screen

    while (1) {
        __asm__ volatile ("hlt");
    }
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

void sse_init() {
    // init SSE
    u32 edx;
    __asm__ volatile (
        "mov $0x1, %%eax\n\t"
        "cpuid\n\t"
        : "=d"(edx)
        :
        : "rax", "rbx", "rcx"
    ); // check if SSE is available otherwise PANIK
    if (!(edx & (1 << 25))) {
        panic("SSE not supported");
    }

    __asm__ volatile (
        "mov %%cr0, %%rax\n\t"
        "and $0xFFFB, %%ax\n\t" // clear coprocessor emulation CR0.EM
        "or $0x2, %%ax\n\t"     // set coprocessor monitoring  CR0.MP
        "mov %%rax, %%cr0\n\t"
        "mov %%cr4, %%rax\n\t"
        "or $0x600, %%ax\n\t" // set CR4.OSFXSR and CR4.OSXMMEXCPT at the same time
        "mov %%rax, %%cr4\n\t"
        :
        :
        : "rax"
    ); // SSE IS ENABLED
}

void paging_init() {
    pml4 = alloc_page();
    u64 *pdpt = alloc_page();
    u64 *pdt = alloc_page();
    u64 *pt = alloc_page();

    memset(pml4, 0, PAGE_SIZE);
    memset(pdpt, 0, PAGE_SIZE);
    memset(pdt, 0, PAGE_SIZE);
    memset(pt, 0, PAGE_SIZE);

    u64 pml4_phys = v2p(pml4);
    u64 pdpt_phys = v2p(pdpt);
    u64 pdt_phys = v2p(pdt);
    u64 pt_phys = v2p(pt);

    pml4[511] = pdpt_phys | 0x03; // present + writable
    pdpt[510] = pdt_phys | 0x03;     // present + writable
    pdt[0] = pt_phys | 0x03;       // present + writable

    for (u64 i = 0; i < 512; i++) {
        pt[i] = (i * PAGE_SIZE) | 0x03; // present + writable
    }

    __asm__ volatile (
        "mov %0, %%cr3\n\t" // load PML4 physical address into CR3
        :
        : "r"(pml4_phys)
        : "memory"
    );
}

void* alloc_page() {
    if (next_free + PAGE_SIZE >= POOL_END) {
        // panic: out of memory in bootstrap pool
        return NULL;
    }
    void* vaddr = (void*)next_free;
    next_free += PAGE_SIZE;
    return vaddr;
}

u64 v2p(void* v) {
    return (u64)v - KERNEL_BASE;
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