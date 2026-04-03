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
    jc disk_error ; if carry flag is set, the disk read is fucked

    ; switch to 32-bit protected mode
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

[BITS 32]
entry32:
    ; Elevate to 64-bit mode
    mov ecx, 0xC0000080 ; IA32_EFER MSR
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ; enable paging
    mov eax, cr0
    or eax, 1 << 31 ; set PG bit
    mov cr0, eax
    
    ; far jump time
    jmp 0x24:entry64 ; jump to 64-bit kernel code segment
end32:
    hlt
    jmp end32

init_pt:
    pushad

    ; Clear the memory area for PAGE TABLES :)
    mov edi, 0x1000
    mov cr3, edi ; store PML4T address in cr3
    xor eax, eax
    mov ecx, 4096
    rep stosd

    ; set edi to PML4T[0]
    mov edi, cr3

    ; set up the first entry of each table
    mov dword [edi], 0x2003 ; Set PML4T[0] to address 0x2000 (PDPT) with flags 0x0003
    add edi, 0x1000 ; move to PDPT[0]
    mov dword [edi], 0x3003 ; Set PDPT[0] to address 0x3000 (PDT) with flags 0x0003
    add edi, 0x1000 ; move to PDT[0]
    mov dword [edi], 0x4003 ; Set PDT[0] to address 0x4000 (PT) with flags 0x0003

    ; fill in the page table
    add edi, 0x1000         ; move to PT[0]
    mov ebx, 0x00000003     ; EBX has address 0x0000 with flags 0x0003
    mov ecx, 512            ; 512 entries to map the first 2MB of memory

    add_page_entry_protected:
        mov dword [edi], ebx ; set page table entry to current address with flags
        add ebx, 0x1000     ; next page (4KB)
        add edi, 8 ; move to next page table entry
        loop add_page_entry_protected

    ; Set up PAE paging, but don't enable it quite yet
    mov eax, cr4
    or eax, 1 << 5 ; set PAE bit
    mov cr4, eax

    popad
    ret

[BITS 64]
entry64:
    mov rdi, 0x0F41
    mov rax, 0xB8000
    mov [rax], rdi ; print 'A' so show that yes I am in 64-bit mode
end64:
    hlt
    jmp end64

section .data
str: db "Sup Homie", 0
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
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt - 1 ; limit
    dd gdt ; base

section .magic
dw 0xAA55
