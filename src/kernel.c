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
    
    // 시스템 콜을 통해 파일 저장 요청
    const char* filename = "test.txt";
    const char* content = "CloudOS File System Test!";
    __asm__ __volatile__("int 0x80" : : "a"(2), "D"(filename), "S"(content), "d"(25));
    
    __asm__ __volatile__("int 0x80" : : "a"(1));
}

void kernel_main(struct multiboot_info* mb_info) {
    vga_init();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("CloudOS Kernel v3.2 (CloudFS)\n");
    vga_print("=============================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    idt_init();
    pmm_init(mb_info->mmap_addr, mb_info->mmap_length);
    vmm_init();
    heap_init();
    
    vga_print("[ OK ] Initializing AHCI Storage...\n");
    ahci_init();
    
    vga_print("[ OK ] Mounting CloudFS...\n");
    vfs_init(); // CloudFS 마운트 (포맷 또는 로드)
    
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
            vga_print("Commands: help, ls, write, read, clear\n");
        } else if (strcmp(cmd, "clear") == 0) {
            vga_init();
        } else if (strcmp(cmd, "ls") == 0) {
            // 파일 목록 출력
            vfs_list_files(); // (vfs.h 및 cloudfs.c에 list 함수 연결 필요)
        } else if (strcmp(cmd, "write") == 0) {
            // 테스트 파일 쓰기
            char* test_data = "This is CloudOS permanent data!";
            vfs_write_file("perm.txt", test_data, 31);
            vga_print("File 'perm.txt' written to disk.\n");
        } else if (strcmp(cmd, "read") == 0) {
            // 테스트 파일 읽기
            char buf[64];
            uint32_t read_bytes = vfs_read_file("perm.txt", buf, 64);
            if (read_bytes > 0) {
                buf[read_bytes] = '\0';
                vga_print("File content: ");
                vga_print(buf);
                vga_print("\n");
            } else {
                vga_print("File not found.\n");
            }
        } else if (idx > 0) {
            vga_print("Unknown command: ");
            vga_print(cmd);
            vga_print("\n");
        }
    }
}
