#include "headers/utils.h"
#include "headers/pmm.h"
#include "headers/vmm.h"
#include "headers/kheap.h"
#include "headers/idt.h"
#include "headers/fdc.h"
#include "headers/elf_parser.h"

void kernel_main(bios_mmap_entry *mmap, u64 mmap_count) __attribute__((section(".kernel")));
void syscall_handler();
void enable_syscall();
void sse_init();
void paging_init();
void relocate_gdt();
void finish_tss_setup();
u64 v2p(void* v);

u16 *VGA_MEMORY = (u16*)(KERNEL_BASE + 0xB8000);

u64 next_free = POOL_START;
u64 *pml4t = NULL;
u8 *mem_bitmap = NULL;
u64 mem_bitmap_size = 0; // in bytes
u64 mem_bitmap_bit_size = 0; // in bits
void *heap_start = 0;
void *heap_end = 0;

extern u64 syscall_stack;

u64 syscall_stack = 0;

void kernel_main(bios_mmap_entry *mmap, u64 mmap_count) {
    // init IDT. Doing it first is for reasons unbeknownst to man
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

    // relocate GDT address of descriptor to higher half
    relocate_gdt();
    finish_tss_setup();

    heap_start = alloc_page();
    for (u8 i = 0; i < 3; i++) {
        heap_end = alloc_page() + PAGE_SIZE;
    }
    heap_init();

    memset((void*)VGA_MEMORY, 0, 80 * 25 * 2); // clear screen

    enable_syscall();
    syscall_stack = ((u64)(alloc_page() + PAGE_SIZE) & -16); // allocate a page for the syscall stack

    // file system testing

    fdc_init();

    while (1) {
        __asm__ volatile ("hlt");
    }

    // TODO move this to dedicated file implementation

    struct task user_program = {0};
    user_program.pid = 1;

    void *tmp = pmm_alloc();
    user_program.pml4t = (u64 *)(tmp + KERNEL_BASE);
    map_page((page_table_t){pml4t, true}, (u64)user_program.pml4t, (u64)tmp, 0x3);
    memcpy(user_program.pml4t, pml4t, 0x1000);

    __asm__ volatile (
        "mov %0, %%cr3"
        :
        : "r"((void *)get_physical_address((page_table_t){pml4t, true}, (u64)user_program.pml4t))
    );

    // load_elf(user_program, (void *)user_program_elf);
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
    pml4t = alloc_page();
    u64 *pdpt = alloc_page();
    u64 *pdt = alloc_page();
    u64 *pt = alloc_page();

    memset(pml4t, 0, PAGE_SIZE);
    memset(pdpt, 0, PAGE_SIZE);
    memset(pdt, 0, PAGE_SIZE);
    memset(pt, 0, PAGE_SIZE);

    u64 pml4_phys = v2p(pml4t);
    u64 pdpt_phys = v2p(pdpt);
    u64 pdt_phys = v2p(pdt);
    u64 pt_phys = v2p(pt);

    pml4t[511] = pdpt_phys | 0x03; // present + writable
    pdpt[510] = pdt_phys | 0x03;     // present + writable
    pdt[0] = pt_phys | 0x03;       // present + writable

    for (u64 i = 0; i < 512; i++) {
        pt[i] = (i * PAGE_SIZE) | 0x03; // present + writable
    }

    __asm__ volatile (
        "mov %0, %%cr3\n\t" // load PML4T physical address into CR3
        :
        : "r"(pml4_phys)
        : "memory"
    );
}

u64 v2p(void* v) { // only use this for kernel addresses otherwise it will return the wrong value
    return (u64)v - KERNEL_BASE;
}