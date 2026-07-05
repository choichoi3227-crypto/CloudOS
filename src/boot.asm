MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0xE85250D6
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 8
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    dw 0
    dw 0
    dd 8

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    cli
    call kernel_main
.hang:
    hlt
    jmp .hang
