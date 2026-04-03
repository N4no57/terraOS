[BITS 32]
init_pt:
    pushad

    ; Clear the memory area for PAGE TABLES :)
    mov edi, 0x1000
    mov cr3, edi ; store PML4T address in cr3
    xor eax, eax
    mov ecx, 4096
    rep stosd

    ; set edi to PML4T[0]
    mov edi, 0x1000
    add edi, 511 * 8 ; move to PML4T[511]

    ; set up the first entry of each table
    mov dword [edi], 0x2003 ; Set PML4T[511] to address 0x2000 (PDPT) with flags 0x0003
    mov dword [edi + 8], 0

    mov edi, 0x2000 ; set to PDPT[0]
    add edi, 510 * 8 ; move to PDPT[510]

    mov dword [edi], 0x3003 ; Set PDPT[510] to address 0x3000 (PDT) with flags 0x0003
    mov dword [edi + 8], 0

    mov edi, 0x3000 ; move to PDT[0]

    mov dword [edi], 0x4003 ; Set PDT[0] to address 0x4000 (PT) with flags 0x0003
    mov dword [edi + 8], 0

    ; fill in the page table
    add edi, 0x1000         ; move to PT[0]
    mov ebx, 0x00000003     ; EBX has address 0x0000 with flags 0x0003
    mov ecx, 512            ; 512 entries to map the first 2MB of memory

    add_page_entry_protected:
        mov dword [edi], ebx ; set page table entry to current address with flags
        mov dword [edi + 4], 0 ; clear upper 32 bits of entry
        add ebx, 0x1000     ; next page (4KB)
        add edi, 8 ; move to next page table entry
        loop add_page_entry_protected

    ; Set up PAE paging, but don't enable it quite yet
    mov eax, cr4
    or eax, 1 << 5 ; set PAE bit
    mov cr4, eax

    popad
    ret