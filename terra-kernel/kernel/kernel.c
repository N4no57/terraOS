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

void kernel_main(void) __attribute__((section(".kernel")));
void* alloc_page();
u64 v2p(void* v);
void* memcpy(void* dest, const void* src, size_t count);

u64 next_free = POOL_START;

void kernel_main(void) {
    // allocate new stack space
    u64 old_rsp;
    __asm__ volatile ("movq %%rsp, %0" : "=r"(old_rsp) : :);
    u64 stack_size = (KERNEL_BASE + 0x9FFF) - old_rsp;
    void *new_stack = alloc_page();
    memcpy(new_stack, (void *)old_rsp, stack_size);

    u16 *VGA_MEMORY = (u16*)0xB8000;
    i32 y = 11;
    const char message[] = "TerraOS - 64-bit C Kernel\0";

    for (i32 i = 0; message[i] != '\0'; i++) {
        VGA_MEMORY[i + y * 80] = 0x0F00 | message[i]; // white text on black background
    }

    for (i32 i = 0; i < 80 * 25; i++) {
        // VGA_MEMORY[i] = 0x0F00 | ' ';
    }

    while (1) {
        __asm__ volatile ("hlt");
    }
}

void* alloc_page() {
    if (next_free + PAGE_SIZE > POOL_END) {
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