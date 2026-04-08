#include "../headers/elf_parser.h"
#include "../headers/vmm.h"
#include "../headers/pmm.h"

static u8 magic[4] = {0x7F, 'E', 'L', 'F'};

u32 load_elf(struct task task, void *elf_data) {
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_data;
    for (u64 i = 0; i < 4; i++) {
        if (ehdr->e_ident[i] != magic[i]) return 0x10; // abort cus magic is wrong
    }

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) return 0x11; // abort cus we want the 64-bit ELF files

    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) return 0x12; // abort cus we want little endian

    if (ehdr->e_machine != EM_X86_64) return 0x13; // abort cus this is an x86-64 machine

    Elf64_Phdr *phdrs = (Elf64_Phdr *)(elf_data + ehdr->e_phoff);

    for (u64 i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD) {
            u64 va_start = phdrs[i].p_vaddr;
            u64 mem_size = phdrs[i].p_memsz;
            u64 f_size   = phdrs[i].p_filesz;
            u64 offset   = phdrs[i].p_offset;
            u64 flags    = phdrs[i].p_flags;

            u64 page_count = (mem_size + 4095) / 4096;
            u16 page_flags = 0x5;
            if ((flags & PF_W) == PF_W && (flags & PF_R) == PF_R) page_flags |= 0x2;
            for (u64 i = 0; i < page_count; i++) {
                void *phys_addr = pmm_alloc();
                map_page((page_table_t){task.pml4t, false}, ((va_start & ~0xFFF) + 0x1000 * i), (u64)phys_addr, page_flags);
            }
        }
    }

    return 0;
}