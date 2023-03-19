    [org 0x7c00]    ; Adding 0x7c00 to labels so that method 2 works but method 3 doesn't

    mov ah, 0x0e

; method 1
    mov al, the_secret
    int 0x10

; method 2
    mov al, [the_secret]
    int 0x10

; method 3
    mov bx, the_secret
    add bx, 0x7c00
    mov al, [bx]
    int 0x10

; method 4
    mov al, [0x7c1e]
    int 0x10

    jmp $

the_secret:
    db "Booting OS",0

    times 510-($-$$) db 0
    dw 0xaa55