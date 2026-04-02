[BITS 16]

section .text
global start

start:
    mov [boot_drive], dl ; save boot drive for later use

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9FFF ; set up stack

    ; read kernel from disk
    mov ah, 0x42 ; extended read specifically in LBA mode
    mov dl, [boot_drive]
    mov si, disk_packet
    int 0x13

    mov si, str
    call print
    hlt

error:
    mov si, disk_error
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

buffer_linear equ 0x100000 ; linear address where the kernel will be loaded (at addr 1MB)

buffer_segment equ buffer_linear >> 4 ; segment:offset for the buffer
buffer_offset equ buffer_linear & 0xF ; offset for the buffer

disk_packet:
    db 0x10 ; size of packet
    db 0 ; reserved
sectors:
    dw 1 ; TODO: number of sectors to read
offset:
    dw buffer_offset ; offset to buffer
segment:
    dw buffer_segment ; segment of buffer
sector:
    dq 0 ; LBA address (Calculated at runtime)

section .magic
dw 0xAA55
