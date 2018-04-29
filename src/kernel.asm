jmp _start

[BITS 16]

_start:
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xffff

    mov ax, 0x100
    mov ds, ax

    call new_line

    mov si, kernel_header_0
    call print_line

    mov si, kernel_header_1
    call print_line

    mov si, kernel_header_2
    call print_line

    call new_line

    jmp $


new_line:
    mov ah, 0Eh

    mov al, 0Ah
    int 10h

    mov al, 0Dh
    int 10h

    ret

print_line:
    mov ah, 0Eh

.repeat:
    lodsb
    cmp al, 0
    je .done
    int 10h
    jmp .repeat

.done:
    call new_line

    ret

key_wait:
    mov al, 0xD2
    out 64h, al

    mov al, 0x80
    out 60h, al

    keyup:
        in al, 0x60
        and al, 10000000b
    jnz keyup
    keydown:
        in al, 0x60

    ret

; Defines

    kernel_header_0 db 'BaLeCoK -> Base Level Computer Kernel', 0
    kernel_header_1 db 'Developed and Maintained by @BTaskaya', 0
    kernel_header_2 db 'Welcome to the our Kernel Part', 0

; Boot Sector
times 512-($-$$) db 0
