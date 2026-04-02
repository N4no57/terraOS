.org 0x7C00
section .text

.global start
start:
    xor ax, ax
    mov si, [str]
    call print
    hlt

print:
    xor bx, bx
    mov ah, 0x0E
    
    mov al, [si]
    cmp al, 0
    je l1

    int 0x10
    inc si
    jmp print

l1: ret

str:
    .asciz "Sup Homie"

.org 510
.word 0xAA55