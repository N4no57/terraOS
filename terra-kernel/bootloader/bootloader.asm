[BITS 16]

section .text
global start

start:
    mov [boot_drive], dl ; save boot drive for later use

    xor ax, ax
    mov si, str
    call print
    hlt

print:
    xor bx, bx
    mov ah, 0x0E
l0:    
    mov al, [si]
    cmp al, 0
    je l1

    int 0x10
    inc si
    jmp l0

l1: ret

section .data
str: db "Sup Homie", 0
disk_error: db "Disk Error", 0

boot_drive: db 0

section .magic
dw 0xAA55