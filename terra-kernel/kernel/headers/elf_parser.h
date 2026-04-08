#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include "utils.h"
#include "elf.h"

typedef u64 Elf64_Addr;
typedef u64 Elf64_Off;
typedef u16 Elf64_Half;
typedef u32 Elf64_Word;
typedef i32 Elf64_Sword;
typedef u64 Elf64_Xword;
typedef i64 Elf64_Sxword;

u32 load_elf(struct task task, void *elf_file);

#endif