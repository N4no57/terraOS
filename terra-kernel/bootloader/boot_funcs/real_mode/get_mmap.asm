[BITS 16]
get_mmap:
    ; get the memory map list from the BIOS and store it at 0x21000 and beyond
    mov ax, 0x2100
    mov es, ax
    xor di, di ; set destination to 0x21000

    xor bx, bx

mmap_loop:
    mov eax, 0xE820
    mov edx, 0x534D4150
    mov ecx, 24
    int 0x15

    jc mmap_end
    cmp eax, 0x534D4150
    jne mmap_end

    add di, 24 ; move forwards by 24 bytes

    inc word [mmap_entry_count]

    test ebx, ebx
    jne mmap_loop
mmap_end:
    ret
