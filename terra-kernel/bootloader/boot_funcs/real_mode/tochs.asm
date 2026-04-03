[BITS 16]
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
