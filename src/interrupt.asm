section .text
bits 64
extern irq_handler_c

global asm_irq_handler
asm_irq_handler:
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
    
    mov rdi, rsp  ; 레지스터 프레임의 주소를 첫 번째 인자로 전달
    call irq_handler_c
    
    ; 컨텍스트 스위칭 후 돌아올 때 스택(rsp)은 새 태스크의 스택으로 바뀌어 있음
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
