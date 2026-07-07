; interrupt.asm
; x86 하드웨어 인터럽트(IRQ)를 C 함수와 연결하는 어셈블리 스텁

section .text

; 외부에 정의된 C 언어 핸들러 함수들
extern keyboard_irq_handler
extern mouse_irq_handler

; IRQ 1 (키보드) 핸들러 스텁
global isr_stub_33
isr_stub_33:
    pushad              ; 레지스터들을 스택에 백업 (상용 OS 필수)
    call keyboard_irq_handler
    mov al, 0x20        ; PIC에게 인터럽트 처리 완료를 알림 (EOI)
    out 0x20, al        ; 마스터 PIC 포트로 전송
    popad               ; 레지스터 복구
    iret                ; 인터럽트 반환

; IRQ 12 (마우스) 핸들러 스텁
global isr_stub_44
isr_stub_44:
    pushad
    call mouse_irq_handler
    mov al, 0x20
    out 0xA0, al        ; 슬레이브 PIC에게 EOI 전송 (마우스는 슬레이브에 연결됨)
    out 0x20, al        ; 마스터 PIC에게 EOI 전송
    popad
    iret
