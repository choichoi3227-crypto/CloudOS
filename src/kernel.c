#include "vga.h"
#include "idt.h"
#include "keyboard.h"
#include "string.h"
#include "pmm.h"
#include "cloudfs_core.h"

// 부트로더에서 전달된 포인터를 직접 받음
void kernel_main(void* mb_info) {
    vga_init();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("CloudOS Core v1.0 (Real Build)\n");
    vga_print("=============================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    idt_init();
    
    // Multiboot2 포인터를 PMM으로 전달하여 메모리 맵 파싱
    pmm_init(mb_info);
    
    vga_print("[ OK ] Mounting CloudFS...\n");
    cloudfs_init();
    
    keyboard_init();
    __asm__ __volatile__("sti");
    
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
            vga_print("Commands: help, ls, write, read\n");
        } else if (strcmp(cmd, "ls") == 0) {
            cloudfs_list_files();
        } else if (strcmp(cmd, "write") == 0) {
            char* data = "This is real data saved to disk!";
            cloudfs_write_file("test.txt", data, 32);
            vga_print("File written to disk.\n");
        } else if (strcmp(cmd, "read") == 0) {
            char buf[64];
            int bytes = cloudfs_read_file("test.txt", buf, 64);
            if (bytes > 0) {
                buf[bytes] = '\0';
                vga_print("File content: ");
                vga_print(buf);
                vga_print("\n");
            } else {
                vga_print("File not found.\n");
            }
        }
    }
}
