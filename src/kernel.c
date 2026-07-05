#include "vga.h"
#include "idt.h"
#include "keyboard.h"
#include "string.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "task.h"
#include "timer.h"
#include "vfs.h"
#include "ahci.h"

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
} __attribute__((packed));

void user_app_main() {
    const char* msg = "[User App] Hello from Ring 3!\n";
    __asm__ __volatile__("int 0x80" : : "a"(0), "D"(msg));
    
    // 파일 시스템에 파일 저장
    const char* filename = "test.txt";
    const char* content = "CloudOS File System Test!";
    __asm__ __volatile__("int 0x80" : : "a"(2), "D"(filename), "S"(content), "d"(25));
    
    __asm__ __volatile__("int 0x80" : : "a"(1));
}

void kernel_main(struct multiboot_info* mb_info) {
    vga_init();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("CloudOS Kernel v3.1 (Storage Layer)\n");
    vga_print("===================================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    idt_init();
    pmm_init(mb_info->mmap_addr, mb_info->mmap_length);
    vmm_init(); // 2GB 1:1 매핑 활성화
    heap_init();
    vfs_init();
    
    vga_print("[ OK ] Initializing AHCI Storage...\n");
    ahci_init(); // SATA 드라이버 초기화
    
    task_init();
    create_user_task(user_app_main);
    timer_init(100);
    
    __asm__ __volatile__("sti");
    
    // 커널 쉘 루프
    while (1) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("admin@cloudos:~$ ");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        
        char cmd[128];
        int idx = 0;
        while (idx < 127) {
            char c = keyboard_getchar();
            if (c == '\n') { vga_print("\n"); break; }
            else if (c == '\b' && idx > 0) { idx--; vga_print("\b \b"); }
            else if (c >= 32 && c <= 126) { cmd[idx++] = c; vga_print_char(c); }
        }
        cmd[idx] = '\0';
        
        if (strcmp(cmd, "help") == 0) {
            vga_print("Commands: help, disk, clear, exit\n");
        } else if (strcmp(cmd, "clear") == 0) {
            vga_init();
        } else if (strcmp(cmd, "disk") == 0) {
            // AHCI 디스크 쓰기/읽기 테스트
            char write_buf[512] = "CloudOS Disk Write Test Data!";
            char read_buf[512];
            
            ahci_write_sectors(100, 1, write_buf); // LBA 100번지에 1섹터 쓰기
            ahci_read_sectors(100, 1, read_buf);   // LBA 100번지에서 1섹터 읽기
            
            vga_print("Disk I/O test completed via AHCI.\n");
        } else if (idx > 0) {
            vga_print("Unknown command: ");
            vga_print(cmd);
            vga_print("\n");
        }
    }
}
