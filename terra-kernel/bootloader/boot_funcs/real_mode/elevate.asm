[BITS 16]
switch_protected_mode:
    ; Disable interrupts
    cli

    lgdt [gdt_descriptor] ; load GDT

    mov eax, cr0
    or eax, 1 ; set PE bit
    mov cr0, eax

    mov ax, 0x10 ; kernel code segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ebp, 0x9000 ; set up fresh stack because YES
    mov esp, ebp

    jmp 8:entry32 ; jump to the loaded kernel