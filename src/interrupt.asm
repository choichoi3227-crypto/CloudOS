section .text
bits 64
extern irq_handler_c

global asm_irq_handler
asm_irq_handler:
    ; 레지스터 저장 (스택 프레임 구성)
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax
    
    mov rdi, rsp  ; 레지스터 프레임 주소 전달
    call irq_handler_c
    
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    iretq
