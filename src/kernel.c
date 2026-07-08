// src/kernel.c
// 역할: OS의 두뇇. 부트로더로부터 제어권을 넘겨받아 하드웨어, 메모리, 파일시스템, GUI를 초기화하고
//       사용자의 입력을 기다리며 무한 루프를 돌림.
// 방식: 상용 OS 표준 부팅 파이프라인 (Init HAL -> Init FS -> Init GUI -> Main Loop)

#include "include/types.h"
#include "include/graphics.h"
#include "include/gpu.h"
#include "include/wm.h"
#include "include/vga.h"
#include "include/idt.h"
include "include/mouse.h"
#include "include/keyboard.h"
#include "mm/heap.h"
#include "sys/bitlocker.h"
#include "fs/vfs.h"
#include "apps/terminal.h"
#include "string.h"

// ==========================================
// 1. 부트로더 정보 구조체 (Multiboot v1 표준 규격)
// GRUB이 화면 세팅 후 이 구조체를 메모리에 채워서 ebx 레지스터에 주소를 넘겨줌
// ==========================================
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;          // 하위 메모리 크기 (KB)
    uint32_t mem_upper;          // 상위 메모리 크기 (KB)
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uintll_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;  // ★ 부트로더가 세팅해 준 화면 메모리의 물리 주소
    uint32_t framebuffer_pitch;  // ★ 한 줄(Scanline)의 바이트 크기 (해상도 * 4바이트)
    uint32_t framebuffer_width;  // ★ 가로 해상도
    uint32_t framebuffer_height; // ★ 세로 해상도
    uint8_t  framebuffer_bpp;    // ★ 픽셀 당 비트 수 (32)
    uint8_t  framebuffer_type;   // ★ 프레임버퍼 타입 (1 = Direct Color/RGB)
} __attribute__((packed)) multiboot_info_t;

// ==========================================
// 2. OS 커널 진입점 (boot.asm에서 호출됨)
// ==========================================
void kernel_main(uint32_t mb_magic, multiboot_info_t* mb_info) {
    // 2-1. 부트로더 검증 및 텍스트 모드 초기화
    vga_init();
    vga_print("========================================\n");
    vga_print("   CloudOS Booting (32-bit PMode)\n");
    vga_print("========================================\n");

    // 부트로더가 제대로 화면을 세팅했는지 확인
    // (flags의 12번째 비트가 1이어야 그래픽 모드 정보가 유효함)
    if (mb_magic != 0x2BADB002 || !(mb_info->flags & (1 << 12))) {
        vga_print("[FATAL ERROR] Bootloader did not set graphics mode!\n");
        vga_print("-> Please add 'set gfxpayload=1024x768x32' to your GRUB config.\n");
        for(;;) asm volatile("hlt"); // 영구 대기
    }
    vga_print("[OK] GRUB initialized graphics mode successfully.\n");

    // 2-2. 하드웨어 추상화 계층(HAL) 초기화
    vga_print("[SYS] Initializing Hardware Abstraction Layer...\n");
    idt_init();           // PIC 리매핑 및 인터럽트 벡터(IDT) 설치
    keyboard_init();      // PS/2 키보드 드라이버 활성화
    mouse_init();         // PS/2 마우스 드라이버 활성화
    vga_print("[OK] Hardware Interrupts Ready.\n");

    // 2-3. 메모리 관리자 초기화 (가상 메모리 할당자 활성화)
    vga_print("[SYS] Initializing Kernel Memory Manager...\n");
    // kmalloc, kfree 함수가 사용 가능해짐 말함 (실제로는 heap.c에서 정적 배열을 할당함)

    // 2-4. 파일 시스템(VFS) 초기화
    vga_print("[SYS] Initializing Virtual File System (RAM Disk)...\n");
    vfs_init();
    // 기본 디렉토리와 테스트 파일 생성
    vfs_create_dir("/bin");
    vfs_create_dir("/sys");
    vfs_create_dir("/apps");
    vfs_create_file("/bin/hello", "Hello from CloudOS VFS!");
    vfs_create_file("/sys/info", "CloudOS Kernel v1.0 2024");
    vfs_create_file("/apps/readme.txt", "CloudOS App Store");
    vga_print("[OK] VFS Mounted.\n");

    // 2-5. 보안 시스템 초기화 (BitLocker 유사 볼륨 암호화)
    vga_print("[SEC] Initializing Volume Encryption...\n");
    static uint8_t virtual_disk[512];
    bitlocker_init_volume(virtual_disk);
    vga_print("[OK] Encryption Layer Ready.\n");

    // 2-6. 그래픽 파이프라인 초기화
    vga_print("[GUI] Initializing Graphics Pipeline...\n");
    gpu_init(); // GPU 하드웨어 스캔 (실패 시 소프트웨어 렌더링으로 자동 폴백됨)
    
    // 부트로더(GRUB)가 알아서 찾아준 화면 메모리를 그래픽 엔진에 연결
    graphics_init(
        (uint32_t *)(uint64_t)mb_info->framebuffer_addr, 
        (int)mb_info->framebuffer_width, 
        (int)mb_info->framebuffer_height, 
        (int)mb_info->framebuffer_pitch
    );
    vga_print("[OK] Graphics Pipeline Ready.\n");

    // 2-7. 윈도우 매니저 및 기본 앱 실행
    vga_print("[SYS] Starting Desktop Environment...\n");
    wm_run(); // 이 함수 안에서 OS가 영원히 루프를 돌며 화면을 유지함
}
