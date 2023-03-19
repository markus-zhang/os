; 3.4.8

    [org 0x7c00]    ; Adding 0x7c00 to labels so that method 2 works but method 3 doesn't

    mov ah, 0x0e

    mov bx, hello_world
    call sub_print
end:
    jmp $
sub_print:
    pusha
    mov al, [bx]
    cmp al, 0
    je  sub_print_end
    int 0x10
    add bx, 1
    jmp sub_print
sub_print_end:
    popa
    ret

hello_world:
    db "Hello, world!",0

    times 510-($-$$) db 0
    dw 0xaa55