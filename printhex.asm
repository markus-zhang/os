    mov dx, 0xffb6;
    mov cl, 12
    mov ax, 0xf000
    pusha
    call print_hex
end:
    jmp $

print_hex:
    mov bx, dx
    and bx, ax
    shr bx, cl          ; only works for CL register if not an immediate number
    cmp bx, 0x0a
    jge over_ten
    add bx, 0x30        ; if 0~9 then add 48 for ASCII '0'~'9'
cont:
    call print
    cmp cl, 0
    je  endsub          ; if ax=0 means all 4 bytes are done
    shr ax, 4           ; shift ax before cl changes
    sub cl, 0x04
    jmp print_hex
over_ten:
    add bx, 0x37        ; if A-F then add 55 for ASCII 'A'~'F'
    jmp cont
endsub:
    popa
    ret
print:
    push ax
    mov ah, 0x0e
    mov al, bl
    int 0x10
    pop ax
    ret

padding:
    times 510-($-$$) db 0
    dw 0xaa55