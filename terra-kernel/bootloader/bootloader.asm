[BITS 16]

section .text
global start

start:
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

section .magic
dw 0xAA55