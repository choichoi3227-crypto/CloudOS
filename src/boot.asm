MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384  ; 16KB 커널 스택
stack_top:

section .text
global _start
extern long_mode_start

_start:
    mov esp, stack_top
    
    ; 32비트 보호 모드에서 64비트 롱 모드로 전환 시도
    call setup_long_mode
    
.hang:
    cli
    hlt
    jmp .hang

; 64비트 모드 설정 루틴
setup_long_mode:
    ; 1. 4단계 페이징을 위한 페이지 테이블 생성 (임시)
    ; (실제로는 long_mode_init.asm에서 수행하고 여기서는 점프)
    jmp long_mode_start
