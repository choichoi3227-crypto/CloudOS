; src/boot.asm (이전 것을 완전히 덮어쓰으세요)
bits 32

section .multiboot
align 4
    dd 0x1BADB002          ; Multiboot v1 매직 넘버
    dd 0x00000003          ; 플래그 (비트 0: 그래픽 모드 지원 요청, 비트 1: 메모리 맵 정보 요청)
    dd -(0x1BADB002 + 0x00000003)

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start
extern kernel_main

_start:
    cli                     ; 인터럽트 비활성화 (안전한 부팅을 위해)
    mov esp, stack_top      ; 스택 설정

    ; Multiboot 규약: 부트로더가 ebx 레지스터에 정보 구조체 주소를 넣어줌
    ; 이 주소를 C언어 kernel_main 함수의 인자로 넘겨줌
    push ebx
    push eax                ; 매직 넘버 (체크용)
    
    call kernel_main
    
    ; kernel_main이 리턴하면 안 되지만, 만약의 패닉 방지용
    cli
    hlt

.hang:
    jmp .hang
