// src/kernel.c
#include "include/types.h"
#include "include/graphics.h"
#include "include/gpu.h"
#include "include/wm.h"
#include "include/vga.h"
#include "include/idt.h"
#include "include/mouse.h"
#include "include/keyboard.h"
#include "mm/heap.h"
#include "sys/bitlocker.h"

// ==========================================
// 상용 OS 부팅 정보 구조체 (Multiboot 표준 규격)
// 부트로더(GRUB)가 이 구조체 형태로 화면 메모리 주소를 넘겨줌
// ==========================================
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;  // ★ 부트로더가 세팅해 준 화면 메모리 주소
    uint32_t framebuffer_pitch;  // ★ 한 줄의 바이트 크기
    uint32_t framebuffer_width;  // ★ 가로 해상도
    uint32_t framebuffer_height; // ★ 세로 해상도
    uint8_t  framebuffer_bpp;    // ★ 픽셀 당 비트 수 (32)
    uint8_t  framebuffer_type;   // ★ 프레임버퍼 타입 (1 = Direct Color/RGB)
} __attribute__((packed)) multiboot_info_t;

// ==========================================
// 커널 진입점 (boot.asm에서 호출됨)
// ==========================================
void kernel_main(uint32_t mb_magic, multiboot_info_t* mb_info) {
    vga_init();
    vga_print("========================================\n");
    vga_print("   CloudOS (32-bit Protected Mode)\n");
    vga_print("========================================\n");

    // 1. 부트로더가 화면을 제대로 세팅했는지 확인
    if (mb_magic != 0x2BADB002 || !(mb_info->flags & (1 << 12))) {
        vga_print("[FATAL ERROR] Bootloader did not set graphics mode!\n");
        vga_print("Please configure GRUB to set gfxpayload=1024x768x32\n");
        for(;;) asm volatile("hlt");
    }

    // 2. 메모리 관리자 초기화
    vga_print("[SYS] Initializing Kernel Heap...\n");

    // 3. 하드웨어 인터럽트 기동
    idt_init();       
    keyboard_init();  
    mouse_init();     

    // 4. 보안 시스템 초기화
    vga_print("[SEC] Initializing Encryption...\n");
    static uint8_t virtual_disk[512];
    bitlocker_init_volume(virtual_disk);

    // 5. 그래픽 파이프라인 초기화 (부트로더가 넘겨준 정보 사용)
    vga_print("[GUI] Initializing Graphics Pipeline...\n");
    gpu_init();
    
    // 부트로더가 찾아준 프레임버퍼를 그래픽 엔진에 연결
    graphics_init(
        (uint32_t *)(uint64_t)mb_info->framebuffer_addr, 
        (int)mb_info->framebuffer_width, 
        (int)mb_info->framebuffer_height, 
        (int)mb_info->framebuffer_pitch
    );

    vga_print("[SYS] Starting Window Manager...\n");

    // 6. GUI 루프 시작
    wm_run();
}
