#include "vga.h"
#include "idt.h"
#include "keyboard.h"
#include "mouse.h"
#include "wm.h"
#include "graphics.h"
#include "string.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "task.h"
#include "timer.h"
#include "vfs.h"
#include "ahci.h"
#include "elf.h"
#include "e1000.h"

struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t dummy[4];
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
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
} __attribute__((packed));

void kernel_main(struct multiboot_info* mb_info) {
    // 1. 그래픽 모드 초기화
    graphics_init(mb_info->framebuffer_addr, mb_info->framebuffer_pitch);
    
    // 텍스트 출력용 (그래픽 모드 위에 텍스트를 그림. 추후 제거됨)
    vga_init(); 
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("CloudOS Kernel v4.0 (GUI Mode)\n");
    vga_print("=============================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    idt_init();
    pmm_init(mb_info->mmap_addr, mb_info->mmap_length);
    vmm_init();
    heap_init();
    
    vga_print("[ OK ] Initializing AHCI Storage...\n");
    ahci_init();
    
    vga_print("[ OK ] Mounting CloudFS...\n");
    vfs_init();
    
    vga_print("[ OK ] Initializing Network...\n");
    e1000_init();
    
    vga_print("[ OK ] Initializing Mouse...\n");
    mouse_init();
    
    task_init();
    timer_init(100);
    
    // GUI 초기화
    wm_init();
    wm_create_window(200, 200, 400, 300, "CloudOS Explorer");
    wm_create_window(600, 150, 300, 200, "Terminal");
    
    __asm__ __volatile__("sti");
    
    // 메인 GUI 루프
    int prev_mx = -1, prev_my = -1, prev_mb = 0;
    while (1) {
        int mx = mouse_get_x();
        int my = mouse_get_y();
        int mb = mouse_get_btn();
        
        // 마우스 상태가 변경되었을 때만 화면 갱신
        if (mx != prev_mx || my != prev_my || mb != prev_mb) {
            // 배경
            draw_rect(0, 0, 1024, 768, 0x1E1E2E);
            
            // 상단 바
            draw_rect(0, 0, 1024, 30, 0x000000);
            draw_string("CloudOS", 10, 8, 0xFFFFFF);
            
            // 하단 Dock
            draw_rect(0, 738, 1024, 30, 0x111111);
            
            // 창 관리자 렌더링
            wm_handle_mouse(mx, my, mb);
            wm_render();
            
            // 마우스 커서 그리기
            draw_rect(mx, my, 10, 10, 0xFFFFFF);
            
            // 버퍼 스왑 (깜빡임 방지)
            swap_buffers();
            
            prev_mx = mx; prev_my = my; prev_mb = mb;
        }
    }
}
