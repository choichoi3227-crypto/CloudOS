; src/boot.asm
bits 32

; Multiboot 헤더 (부트로더가 이 파일을 인식하게 함)
section .multiboot
align 4
    dd 0x1BADB002       ; 매직 넘버
    dd 0x00             ; 플래그
    dd -(0x1BADB002 + 0x00) ; 체크섬

section .bss
align 16
stack_bottom:
    resb 16384 ; 16KB 커널 스택
stack_top:

section .text
global _start
extern kernel_main

_start:
    ; 스택 포인터 설정
    mov esp, stack_top

    ; ==========================================
    ; VBE 그래픽 모드 설정 (1024x768x32bit)
    ; ==========================================
    mov ax, 0x4F02       ; VBE Set Mode 함수
    mov bx, 0x4118       ; 1024x768, 32비트 색상 (Direct Color), LFB 사용
    int 0x10             ; BIOS 비디오 인터럽트 호출

    ; 설정된 모드 정보 가져오기
    mov ax, 0x4F01
    mov cx, 0x118        ; 요청한 모드 번호
    int 0x10

    cmp ax, 0x004F       ; 지원 여부 확인
    jne .fail_graphic

    ; VBE 정보 구조체에서 프레임버퍼 정보 추출
    ; ES:DI에 정보가 들어옴 (Multiboot 규격상 전역 변수로 밀어 넣음)
    mov eax, [es:di + 40] ; physical_address (프레임버퍼 주소)
    mov [boot_fb_addr], eax
    mov eax, [es:di + 16] ; bytes_per_scan_line (Pitch)
    mov [boot_fb_pitch], eax
    mov eax, [es:di + 12] ; width
    mov [boot_fb_width], eax
    mov eax, [es:di + 14] ; height
    mov [boot_fb_height], eax

    jmp kernel_main

.fail_graphic:
    ; 그래픽 모드 실패 시 멈춤 (실사용 OS는 여기서 텍스트 모드로 폴백해야 함)
    hlt
    jmp .fail_graphic

section .data
boot_fb_addr: dd 0
boot_fb_pitch: dd 0
boot_fb_width: dd 0
boot_fb_height: dd 0
