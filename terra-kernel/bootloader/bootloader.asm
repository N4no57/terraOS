[BITS 16]

; BPB pointers
BYTES_PER_SECTOR equ 11 ; 2 bytes
SECTORS_PER_CLUSTER equ 13 ; 1 byte
RESERVED_SECTORS equ 14 ; 2 bytes
NUM_FATS equ 16 ; 1 byte
ROOT_ENTRIES equ 17 ; 2 bytes
TOTAL_SECTORS equ 19 ; 2 bytes
NUMBER_OF_SECTORS_PER_FAT equ 22 ; 2 bytes
SECTORS_PER_TRACK equ 24 ; 2 bytes
NUM_HEADS equ 26 ; 2 bytes
HIDDEN_SECTORS equ 28 ; 4 bytes
LARGE_SECTOR_COUNT equ 32 ; 4 bytes

section .text
global start

start:
    mov [boot_drive], dl ; save boot drive for later use

    mov ax, 0x1000
    mov es, ax ; set ES to 0x1000 for loading kernel
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x9000 ; set up stack

    call ToCHS ; convert LBA to CHS for disk read

    ; read kernel from disk
    mov ah, 0x2 ; extended read specifically in LBA mode
    mov al, 1 ; read 1 sector
    mov ch, [track_var] ; cylinder
    mov cl, [sector_var] ; sector
    mov dh, [head_var] ; head
    mov dl, [boot_drive]
    xor bx, bx ; buffer offset in ES:BX
    int 0x13
    jc disk_error ; if carry flag is set, the disk read is fucked

    mov si, str
    call print
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
    mov [sector_var], ax         ; if DX points to sector variable

    ; store head and track
    mov [head_var], di         ; if BX points to head variable
    mov [track_var], si         ; if CX points to track variable
    ret

section .data
str: db "Sup Homie", 0
disk_error_str: db "Disk Error", 0

boot_drive: db 0

head_var: dw 0
track_var: dw 0
sector_var: dw 0

section .magic
dw 0xAA55
