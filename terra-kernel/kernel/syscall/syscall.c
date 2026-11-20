#include "../headers/utils.h"

void syscall_handler() {
    // currently does nothing
    static char syscall_message[] = "syscall has occurred :O";

    for (u8 i = 0; syscall_message[i] != '\0'; i++) {
        VGA_MEMORY[i] = 0x0F00 | syscall_message[i];
    }

    while (1) {
        __asm__ volatile ("hlt");
    }
}

extern void syscall_stub();

void enable_syscall() {
    // enable syscall and sysret
    u64 IA32_EFER_VAL = read_msr(IA32_EFER);
    IA32_EFER_VAL |= 1;
    write_msr(IA32_EFER, IA32_EFER_VAL);

    // set syscall segments
    u64 STAR_MSR_VAL = (0x18ull << 32ull) | (0x28ull << 48ull) | 0;
    write_msr(STAR_MSR, STAR_MSR_VAL);

    // set syscall subroutine address
    u64 LSTAR_MSR_VAL = (u64)&syscall_stub;
    write_msr(LSTAR_MSR, LSTAR_MSR_VAL);

    // set flags reg mask to 0
    u64 SFMASK_MSR_VAL = 0;
    write_msr(SFMASK_MSR, SFMASK_MSR_VAL);
}