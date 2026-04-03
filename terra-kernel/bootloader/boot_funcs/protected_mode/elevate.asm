[BITS 32]
switch_long_mode:
    mov ecx, 0xC0000080 ; IA32_EFER MSR
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ; enable paging
    mov eax, cr0
    or eax, 1 << 31 ; set PG bit
    mov cr0, eax
    
    ; far jump time
    jmp 0x24:entry64 ; jump to 64-bit kernel code segment