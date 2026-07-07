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

// boot.asm에서 넘겨주는 프레임버퍼 변수들
extern uint32_t boot_fb_addr;
extern uint32_t boot_fb_pitch;
extern uint32_t boot_fb_width;
extern uint32_t boot_fb_height;

// 가상의 디스크 메모리 (실제론 AHCI/IDE 드라이버가 읽음)
static uint8_t virtual_disk[512];

void kernel_main(void) {
    vga_init();
    vga_print("========================================\n");
    vga_print("   CloudOS (Production Build) Booting   \n");
    vga_print("========================================\n");

    // 1. 메모리 관리자 초기화 (kmalloc 활성화)
    vga_print("[SYS] Initializing Kernel Heap...\n");
    // heap_init()은 정적 배열이므로 별도 초기화 불필요

    // 2. 하드웨어 인터럽트 시스템 기동
    idt_init();       
    keyboard_init();  
    mouse_init();     

    // 3. 보안/암호화 시스템 초기화 (BitLocker 유사 기술)
    vga_print("[SEC] Initializing CloudOS Volume Encryption...\n");
    bitlocker_init_volume(virtual_disk);
    // 실제 환경에서는 부팅 시 "암호를 입력하세요" GUI가 뜸

    // 4. 그래픽 파이프라인 초기화
    vga_print("[GUI] Initializing Graphics Pipeline...\n");
    if (boot_fb_addr == 0) {
        vga_print("[FATAL ERROR] No Framebuffer from Bootloader!\n");
        for(;;) asm volatile("hlt");
    }

    gpu_init();
    graphics_init(
        (uint32_t *)boot_fb_addr, 
        (int)boot_fb_width, 
        (int)boot_fb_height, 
        (int)boot_fb_pitch
    );

    vga_print("[SYS] Starting Window Manager...\n");

    // 5. 윈도우 매니저 실행 (무한 루프)
    wm_run();
}
