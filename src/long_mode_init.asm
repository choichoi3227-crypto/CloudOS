section .text
bits 32
global long_mode_start
extern kernel_main

long_mode_start:
    ; 페이지 테이블 생성 (간략화된 1:1 매핑)
    mov eax, p4_table
    or eax, 0b11        ; present + writable
    mov [p4_table + 0], eax
    
    mov eax, p3_table
    or eax, 0b11
    mov [p3_table + 0], eax

    mov eax, p2_table
    or eax, 0b11
    mov [p2_table + 0], eax

    ; p2 테이블에 2MB 페이지 매핑 (최대 2GB까지 1:1 매핑)
    mov ecx, 0
.map_p2_table:
    mov eax, 0x200000
    mul ecx
    or eax, 0b10000011  ; huge page + present + writable
    mov [p2_table + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jne .map_p2_table

    ; CR3 레지스터에 P4 테이블 주소 로드
    mov eax, p4_table
    mov cr3, eax

    ; PAE (Physical Address Extension) 활성화
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Long Mode 활성화 (MSR 설정)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; 페이징 활성화 (CR0)
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; 64비트 GDT 로드
    lgdt [gdt64.pointer]

    ; 64비트 코드 세그먼트로 점프
    jmp gdt64.code:long_mode_start_64

section .bss
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096

section .rodata
gdt64:
    dq 0 ; null
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; code segment
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .text
bits 64
long_mode_start_64:
    ; 데이터 세그먼트 초기화
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; C 커널 진입
    call kernel_main

    hlt
