; demonstration of segment register

    mov ah, 0x0e

    mov al, [the_secret]
    int 0x10

    mov bx, 0x7c0
    mov ds, bx              ; cannot move immediate to segment register
    mov al, [the_secret]
    int 0x10

    mov al, [ds:the_secret]
    int 0x10

    mov bx, 0x7c0
    mov es, bx          
    mov al, [es:the_secret]
    int 0x10

    jmp $

the_secret:
    db "X"

; Padding
    times 510-($-$$) db 0
    dw 0xaa55