section .text
bits 64
extern irq_handler_c

; 공통 인터럽트 핸들러 (에러 코드가 없는 경우 호출됨)
global asm_common_irq_handler
asm_common_irq_handler:
    ; 가짜 에러 코드 0 푸시 (스택 프레임 통일)
    push 0
    ; 인터럽트 번호는 매크로를 통해 푸시됨
    ; 레지스터 저장
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
    
    mov rdi, rsp
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
    
    ; 에러 코드와 인터럽트 번호 제거
    add rsp, 16
    iretq

; 에러 코드가 있는 인터럽트 핸들러 (페이지 폴트 등)
global asm_err_irq_handler
asm_err_irq_handler:
    ; 에러 코드는 이미 스택에 있음
    push 0 ; 인터럽트 번호 자리 (매크로로 채움)
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
    
    mov rdi, rsp
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
    
    add rsp, 16
    iretq

; %1: 인터럽트 번호, %2: 핸들러 이름
%macro DEFINE_IRQ 2
global %2
%2:
    push %1           ; 인터럽트 번호 푸시
    jmp asm_common_irq_handler
%endmacro

; 32~47번 인터럽트(타이머, 키보드 등) 핸들러 생성
DEFINE_IRQ 32, irq32
DEFINE_IRQ 33, irq33
DEFINE_IRQ 34, irq34
DEFINE_IRQ 35, irq35
DEFINE_IRQ 36, irq36
DEFINE_IRQ 37, irq37
DEFINE_IRQ 38, irq38
DEFINE_IRQ 39, irq39
DEFINE_IRQ 40, irq40
DEFINE_IRQ 41, irq41
DEFINE_IRQ 42, irq42
DEFINE_IRQ 43, irq43
DEFINE_IRQ 44, irq44
DEFINE_IRQ 45, irq45
DEFINE_IRQ 46, irq46
DEFINE_IRQ 47, irq47
