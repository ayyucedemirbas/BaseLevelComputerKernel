int_str_length:
    push rbx
    push rdx
    push rsi

    mov rax, r8

    xor rsi, rsi
        mov rbx, 10
        div rbx
        add rdx, 48

        inc rsi

        test rax, rax
        jne .loop


    .loop:
        xor rdx, rdx
        mov rbx, 10
        div rbx
        add rdx, 48

        inc rsi

        cmp rax, 0
        jne .loop

    .exit:
        mov rax, rsi

        pop rsi
        pop rdx
        pop rbx

        ret
        
