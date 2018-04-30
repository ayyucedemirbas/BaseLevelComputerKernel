install_timer:
    mov r8, 1193180 / 1000

    mov al, 0x36
    out 0x43, al 

    mov rax, r8
    out 0x40, al

    mov rax, r8
    shr rax, 8
    out 0x40, al

    mov r8, 0
    mov r9, irq_timer_handler
    call register_irq_handler

    ret

irq_timer_handler:
    push rax

    mov rax, [timer_ticks]
    inc rax
    mov [timer_ticks], rax

    xor rdx, rdx
    mov rcx, 1000
    div rcx
    cmp rdx, 0
    jnz .end

    mov rax, [timer_seconds]
    inc rax
    mov [timer_seconds], rax

    .end:

    pop r8

    ret


timer_ticks dq 0
timer_seconds dq 0
