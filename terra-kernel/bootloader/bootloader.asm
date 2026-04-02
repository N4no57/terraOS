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
    mov ss, ax
    mov sp, 0x9000 ; set up stack

    mov ax, 0x1000
    mov es, ax ; set ES to 0x1000 for loading kernel

    ; calculate root directory sectors
    mov ax, [ROOT_ENTRIES]
    mov bx, 32
    mul bx ; BPB_RootEntCnt × 32
    mov bx, [BYTES_PER_SECTOR]
    sub bx, 1 ; BytesPerSector - 1
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
    mov ah, 0x2 ; extended read specifically in LBA mode
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
    mov ah, 0x2 ; extended read specifically in LBA mode
    mov al, 1 ; read 1 sector 
    mov ch, [track_var] ; cylinder
    mov cl, [sector_var] ; sector
    mov dh, [head_var] ; head
    mov dl, [boot_drive] ; drive number
    xor bx, bx ; buffer offset in ES:BX
    int 0x13
    mov si, disk_error_str
    mov byte [si], 0x4D
    jc disk_error ; if carry flag is set, the disk read is fucked

    jmp far [kernel_ptr] ; jump to the loaded kernel

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

; Input:
;   AX = LBA
ToCHS:
    mov bx, [SECTORS_PER_TRACK]
    shl bx, 1            ; SECTORS_PER_TRACK * 2
    xor dx, dx           ; DX = 0 for division

    ; Calculate track = LBA / (SECTORS_PER_TRACK*2)
    div bx               ; AX / BX -> AX=quotient(track), DX=remainder
    mov si, ax           ; save track in SI

    ; Calculate head = (LBA % (SPT*2)) / SPT
    mov ax, dx           ; remainder from previous division
    mov bx, [SECTORS_PER_TRACK]
    xor dx, dx
    div bx               ; AX / BX -> AX=quotient(head), DX=remainder
    mov di, ax           ; head

    ; Calculate sector = (LBA % SECTORS_PER_TRACK) + 1
    mov ax, dx           ; remainder from head division
    add ax, 1
    ; store sector
    mov [sector_var], ax ; store sector

    ; store head and track
    mov [head_var], di ; store head
    mov [track_var], si ; store track
    ret

section .data
str: db "Sup Homie", 0
disk_error_str: db "Disk Error", 0

kernel_ptr:
    dw 0x0000 ; offset
    dw 0x1000 ; segment

boot_drive: db 0
root_dir_start: dw 0
root_dir_size: dw 0
data_sector_start: dw 0
sectors_to_read: dw 0

head_var: dw 0
track_var: dw 0
sector_var: dw 0

section .magic
dw 0xAA55
