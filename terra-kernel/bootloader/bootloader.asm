[BITS 16]

; BPB pointers
BYTES_PER_SECTOR equ 0x7C0B ; 2 bytes
SECTORS_PER_CLUSTER equ 0x7C0D ; 1 byte
RESERVED_SECTORS equ 0x7C0E ; 2 bytes
NUM_FATS equ 0x7C10 ; 1 byte
ROOT_ENTRIES equ 0x7C11 ; 2 bytes
TOTAL_SECTORS equ 0x7C13 ; 2 bytes
NUMBER_OF_SECTORS_PER_FAT equ 0x7C16 ; 2 bytes
SECTORS_PER_TRACK equ 0x7C18 ; 2 bytes
NUM_HEADS equ 0x7C1A ; 2 bytes
HIDDEN_SECTORS equ 0x7C1C ; 4 bytes
LARGE_SECTOR_COUNT equ 0x7C20 ; 4 bytes

section .text
global start

start:
    mov [boot_drive], dl ; save boot drive for later use

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000 ; set up stack

    ; load the extended bootloader from disk into memory at 0x7E00
    mov ax, 1
    call ToCHS ; convert LBA 1 to CHS for disk read

    mov ah, 0x2
    mov al, 1 ; read 1 sector
    mov ch, [track_var] ; cylinder
    mov cl, [sector_var] ; sector
    mov dh, [head_var] ; head
    mov dl, [boot_drive] ; drive number
    mov bx, 0x7E00 ; buffer offset in ES:BX
    int 0x13
    jc disk_error ; if carry flag is set, the disk read is fucked otherwise we have the extended bootloader in memory at 0x7E00 and we are not fucked

    mov ax, 0x1000
    mov es, ax ; set ES to 0x1000 for loading kernel

    ; calculate root directory sectors
    mov ax, [ROOT_ENTRIES]
    mov bx, 32
    mul bx ; BPB_RootEntCnt × 32
    mov bx, [BYTES_PER_SECTOR]
    dec bx ; BytesPerSector - 1
    add ax, bx ; (BPB_RootEntCnt × 32) + (BytesPerSector - 1)
    xor dx, dx
    div word [BYTES_PER_SECTOR] ; ((BPB_RootEntCnt × 32) + (BytesPerSector - 1)) / BytesPerSector
    mov [root_dir_size], ax

    ; calculate first data sector
    mov al, [NUM_FATS]
    mul word [NUMBER_OF_SECTORS_PER_FAT]
    add ax, [RESERVED_SECTORS]
    mov [root_dir_start], ax
    add ax, [root_dir_size]
    mov [data_sector_start], ax

    mov ax, [root_dir_start]
    call ToCHS ; convert root directory LBA to CHS for disk read

    ; read kernel directory entry from disk
    mov ah, 0x2
    mov al, 1 ; read 1 sector
    mov ch, [track_var] ; cylinder
    mov cl, [sector_var] ; sector
    mov dh, [head_var] ; head
    mov dl, [boot_drive] ; drive number
    xor bx, bx ; buffer offset in ES:BX
    int 0x13
    jc disk_error ; if carry flag is set, the disk read is fucked

    mov ax, [es:bx + 0x3A] ; starting cluster of the kernel file
    sub ax, 2 ; cluster numbers start at 2
    add ax, [data_sector_start] ; LBA of the kernel's first data sector

    call ToCHS ; convert kernel LBA to CHS for disk read

    ; read kernel from disk
    mov ah, 0x2
    mov al, 127 ; read 127 sectors as that is ~ 64KB which is how much the page table will allocate
    mov ch, [track_var] ; cylinder
    mov cl, [sector_var] ; sector
    mov dh, [head_var] ; head
    mov dl, [boot_drive] ; drive number
    xor bx, bx ; buffer offset in ES:BX
    int 0x13
    jc disk_error ; if carry flag is set, the disk read is fucked

    ; switch to 32-bit protected mode
    jmp switch_protected_mode

end:
    hlt
    jmp end

disk_error:
    mov si, disk_error_str
    call print
    jmp end

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

; 16-bit includes
%include "terra-kernel/bootloader/boot_funcs/real_mode/tochs.asm" ; this is required to load in the extended bootloader
section .extended_bootloader
%include "terra-kernel/bootloader/boot_funcs/real_mode/elevate.asm"

[BITS 32]
entry32:
    call init_pt ; set up page tables for long mode
    ; Elevate to 64-bit mode
    call switch_long_mode
end32:
    hlt
    jmp end32

; 32-bit includes
%include "terra-kernel/bootloader/boot_funcs/protected_mode/init_pt.asm"
%include "terra-kernel/bootloader/boot_funcs/protected_mode/elevate.asm"

[BITS 64]
entry64:
    mov rsp, 0xFFFFFFFF80000000 + 0x20000 + 0xFFF ; set the stack at the top of the 2MB page we mapped in protected mode
    and rsp, -16
    sub rsp, 8
    mov rbp, rsp

    mov rax, 0xFFFFFFFF80000000 + 0x10000
    jmp rax
end64:
    hlt
    jmp end64

section .data
disk_error_str: db "Disk Error", 0

boot_drive: db 0
root_dir_start: dw 0
root_dir_size: dw 0
data_sector_start: dw 0
sectors_to_read: dw 0

head_var: dw 0
track_var: dw 0
sector_var: dw 0

gdt:
    dq 0 ; null descriptor
    dq 0x00CF9A000000FFFF ; 32-bit kernel code segment descriptor
    dq 0x00CF92000000FFFF ; kernel data segment descriptor
    dq 0x00AF9A000000FFFF ; 64-bit kernel code segment descriptor
    dq 0x00CFF2000000FFFF ; user data segment descriptor
    dq 0x00AFFA000000FFFF ; user code segment descriptor
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt - 1 ; limit
    dd gdt ; base

section .magic
dw 0xAA55
