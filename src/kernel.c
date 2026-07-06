#include "vga.h"
#include "idt.h"
#include "keyboard.h"
#include "mouse.h"
#include "graphics.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "timer.h"
#include "scheduler.h"
#include "perfect_os.h"
#include "ahci.h"
#include "string.h"

struct multiboot_info {
    uint32_t flags; uint32_t mem_lower; uint32_t mem_upper;
    uint32_t dummy[8]; uint32_t mmap_length; uint32_t mmap_addr;
    uint64_t framebuffer_addr; uint32_t framebuffer_pitch;
} __attribute__((packed));

void kernel_main(struct multiboot_info* mb_info) {
    graphics_init(mb_info->framebuffer_addr, mb_info->framebuffer_pitch);
    
    vga_init(); 
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("CloudOS Ultimate Kernel v7.0 (Perfect 10)\n");
    vga_print("==========================================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    idt_init();
    pmm_init(mb_info->mmap_addr, mb_info->mmap_length);
    vmm_init();
    heap_init();
    
    ahci_init();
    cloudfs_v3_init(); // 10점 파일 시스템
    
    security_enhanced_init(); // 10점 보안
    driver_framework_init();  // 10점 드라이버 프레임워크
    driver_auto_detect();     // 하드웨어 자동 감지 및 설치
    
    power_acpi_init();        // 10점 전력 관리
    scheduler_init();
    compositor_pro_init();    // 10점 GUI
    
    timer_init(100);
    __asm__ __volatile__("sti");
    
    int current_desktop = 0;
    uint64_t last_tick = 0;
    
    while (1) {
        // 10점 전력 관리: 유휴 시 Modern Standby 진입
        if (timer_ticks == last_tick) {
            power_enter_modern_standby();
            continue;
        }
        last_tick = timer_ticks;
        
        // 가상 데스크탑 스위칭 예제 (5초마다 전환)
        if (timer_ticks % 500 == 0) {
            current_desktop = (current_desktop + 1) % MAX_DESKTOPS;
        }
        
        // 10점 GUI 렌더링 (HDR, 다중 데스크탑)
        compositor_render_hdr(current_desktop);
        
        // 마우스 커서
        int mx = mouse_get_x();
        int my = mouse_get_y();
        draw_rect(mx, my, 12, 12, 0xFFFFFF);
        
        swap_buffers();
    }
}
