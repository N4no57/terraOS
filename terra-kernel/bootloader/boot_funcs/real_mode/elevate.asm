[BITS 16]
switch_protected_mode:
    ; Disable interrupts
    cli

    ; do the GDT TSS entry
    mov si, gdt + 0x30
    mov bx, tss_end - tss - 1
    mov [si], bx
    add si, 0x2 ; move to limit base low
    mov bx, tss
    mov [si], bx
    add si, 0x2 ; move to base middle
    mov bl, 0
    mov [si], bl
    inc si ; move to access byte
    mov bl, 0x89
    mov [si], bl
    inc si ; move to limit middle
    mov bl, 0
    mov [si], bl
    inc si ; move to base high
    mov bl, 0x80
    mov [si], bl
    inc si ; move to base upper (low 16-bits)
    mov bx, 0xFFFF
    mov [si], bx
    add si, 0x2
    mov [si], bx

    ; set IOBP to sizeof(TSS)
    mov si, tss + 0x66
    mov bx, tss_end - tss - 1
    mov [si], bx

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

    mov ebp, 0x90000 ; set up fresh stack because YES
    mov esp, ebp

    jmp 8:entry32 ; jump to the loaded kernel