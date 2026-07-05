section .multiboot
align 8
header_start:
    dd 0xe85250d6                ; magic
    dd 0                         ; architecture
    dd header_end - header_start ; header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; checksum
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .rodata
gdt64:
    dq 0
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top

    ; 4단계 페이징을 위한 임시 테이블 생성 (1GB 매핑)
    mov eax, p3_table
    or eax, 0b11
    mov [p4_table], eax

    mov eax, p2_table
    or eax, 0b11
    mov [p3_table], eax

    mov ecx, 0
.map_p2:
    mov eax, 0x200000
    mul ecx
    or eax, 0b10000011
    mov [p2_table + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jne .map_p2

    ; CR3 로드
    mov eax, p4_table
    mov cr3, eax

    ; PAE 활성화
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Long Mode 활성화
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; 페이징 활성화
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; GDT 로드 후 64비트 점프
    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode_start

section .bss
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096

section .text
bits 64
long_mode_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call kernel_main

    cli
.hang:
    hlt
    jmp .hang
