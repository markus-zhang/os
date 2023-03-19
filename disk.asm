    mov ah, 0x02            ; BIOS read sector function

    mov dl, 0               ; read drive 0
    mov ch, 3               ; select cylinder 3
    mov dh, 1               ; select the track on 2nd side of floppy disk, base is 0
    mov al, 5               ; read five sectors from the start point

; set the address that we'd like BIOS to read the sectors to, which BIOS expects
; to find in ES:BX (segment ES with offset BX)
    mov bx, 0xa000
    mov es, bx
    mov bx, 0x1234
; above translates to physical address 0xa0000+0x1234=0xa1234

    int 0x13                ; issue the BIOS interrupt to do the actual read

; if BIOS failed to read, it set CF to signal a general fault, 
; and set al to the number of sectors actually read
    jc disk_error

    cmp al, 5               ; check whether 5 sectors have been read
    jne disk_error

disk_error:
    mov bx, DISK_ERROR_MSG
    call print_string
    jmp $

print_string:
    push bx                 ; backup bx
    mov ah, 0x0e            ; setup BIOS

    mov bx, 0x7c0
    mov es, bx              ; setup segment register
 
    pop bx                  ; restore bx to DISJ_ERROR_MSG
    mov al, [es:bx]
    cmp al, 0
    je  print_string_end
    int 0x10
    add bx, 1
    jmp print_string
print_string_end:
    ret

; global variables
DISK_ERROR_MSG:
    db "Disk read error!", 0

    times 510-($-$$) db 0
    dw 0xaa55