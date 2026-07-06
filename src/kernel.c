#include "vga.h"
#include "idt.h"
#include "keyboard.h"
#include "mouse.h"
#include "wm.h"
#include "graphics.h"
#include "usb.h"
#include "acpi.h"
#include "audio.h"
#include "security.h"
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

// ... (기존 multiboot_info 구조체 유지) ...

void kernel_main(struct multiboot_info* mb_info) {
    graphics_init(mb_info->framebuffer_addr, mb_info->framebuffer_pitch);
    
    vga_init(); 
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("CloudOS Kernel v4.4 (Security Model)\n");
    vga_print("====================================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    idt_init();
    pmm_init(mb_info->mmap_addr, mb_info->mmap_length);
    vmm_init();
    heap_init();
    
    ahci_init();
    vfs_init();
    e1000_init();
    usb_init();
    acpi_init();
    audio_init();
    
    vga_print("[ OK ] Initializing Security...\n");
    security_init();
    
    // 간이 로그인 (자동 로그인: user 계정)
    vga_print("[ OK ] Auto-login as 'user' (UID: 1).\n");
    security_login(1);
    
    mouse_init();
    task_init();
    timer_init(100);
    
    wm_init();
    wm_create_window(200, 200, 400, 300, "CloudOS Explorer");
    wm_create_window(600, 150, 300, 200, "Terminal");
    
    __asm__ __volatile__("sti");
    
    int prev_mx = -1, prev_my = -1, prev_mb = 0;
    uint64_t last_timer_tick = 0;
    
    while (1) {
        int mx = mouse_get_x();
        int my = mouse_get_y();
        int mb = mouse_get_btn();
        int wheel = mouse_get_wheel();
        
        if (mx != prev_mx || my != prev_my || mb != prev_mb || wheel != 0 || timer_ticks != last_timer_tick) {
            draw_rect(0, 0, 1024, 768, 0x1E1E2E);
            
            draw_rect(0, 0, 1024, 30, 0x000000);
            draw_string("CloudOS", 10, 8, 0xFFFFFF);
            
            // 현재 로그인된 사용자 표시
            user_context_t* user = security_get_current_user();
            if (user) {
                draw_string("User: ", 200, 8, 0xFFFFFF);
                draw_string(user->username, 240, 8, 0x27C93F);
            }
            
            draw_string("BAT: 100%", 1024 - 150, 8, 0x27C93F);
            
            uint64_t secs = timer_ticks / 100;
            uint64_t mins = secs / 60;
            uint64_t hours = (mins / 60) % 24;
            mins = mins % 60;
            secs = secs % 60;
            
            draw_rect(1024 - 250, 8, 100, 16, 0x000000);
            draw_number(hours, 1024 - 250, 8, 0xFFFFFF);
            draw_string(":", 1024 - 250 + 16, 8, 0xFFFFFF);
            draw_number(mins, 1024 - 250 + 24, 8, 0xFFFFFF);
            draw_string(":", 1024 - 250 + 40, 8, 0xFFFFFF);
            draw_number(secs, 1024 - 250 + 48, 8, 0xFFFFFF);
            
            draw_rect(0, 738, 1024, 30, 0x111111);
            draw_rect(10, 743, 20, 20, 0x4A90E2);
            
            wm_handle_mouse(mx, my, mb);
            wm_render();
            
            draw_rect(mx, my, 12, 12, 0xFFFFFF);
            draw_rect(mx+2, my+2, 8, 8, 0x000000);
            
            swap_buffers();
            
            prev_mx = mx; prev_my = my; prev_mb = mb;
            last_timer_tick = timer_ticks;
        }
    }
}
