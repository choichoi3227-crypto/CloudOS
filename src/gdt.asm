section .text
bits 64
global gdt_load_tss

gdt_load_tss:
    lgdt [rdi]
    mov ax, 0x28
    ltr ax
    ret
