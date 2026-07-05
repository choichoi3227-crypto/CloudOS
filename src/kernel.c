#include "vga.h"
#include "idt.h"
#include "keyboard.h"
#include "string.h"

void kernel_main() {
    vga_init();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("CloudOS Kernel v1.0 (x86_64 Native)\n");
    vga_print("====================================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    vga_print("[ OK ] Initializing IDT & PIC...\n");
    idt_init();
    
    vga_print("[ OK ] Initializing Keyboard Driver...\n");
    keyboard_init();
    
    __asm__ __volatile__("sti"); // 인터럽트 활성화
    
    vga_print("[ OK ] System Ready.\n\n");

    while (1) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_print("CloudOS Login\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        
        vga_print("User: admin\n");
        vga_print("Pass: ");
        
        char password[32];
        int idx = 0;
        while (idx < 31) {
            char c = keyboard_getchar();
            if (c == '\n') {
                vga_print("\n");
                break;
            } else if (c == '\b' && idx > 0) {
                idx--;
                vga_print("\b \b");
            } else if (c >= 32 && c <= 126) {
                password[idx++] = c;
                vga_print_char('*');
            }
        }
        password[idx] = '\0';
        
        if (strcmp(password, "password") == 0) {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_print("\nLogin Success! Welcome to CloudOS.\n\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            break;
        } else {
            vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
            vga_print("\nLogin Failed! Try again.\n\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
    }

    vga_print("Type 'help' for commands.\n");
    while (1) {
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_print("admin@cloudos:~$ ");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        
        char cmd[128];
        int idx = 0;
        while (idx < 127) {
            char c = keyboard_getchar();
            if (c == '\n') {
                vga_print("\n");
                break;
            } else if (c == '\b' && idx > 0) {
                idx--;
                vga_print("\b \b");
            } else if (c >= 32 && c <= 126) {
                cmd[idx++] = c;
                vga_print_char(c);
            }
        }
        cmd[idx] = '\0';
        
        if (strcmp(cmd, "help") == 0) {
            vga_print("Available commands: help, clear, about, exit\n");
        } else if (strcmp(cmd, "clear") == 0) {
            vga_init();
        } else if (strcmp(cmd, "about") == 0) {
            vga_print("CloudOS Perfect Edition (Native)\nArch: x86_64 | GUI: Pending\n");
        } else if (strcmp(cmd, "exit") == 0) {
            vga_print("System Halted.\n");
            __asm__ __volatile__("cli; hlt");
        } else if (idx > 0) {
            vga_print("Unknown command: ");
            vga_print(cmd);
            vga_print("\n");
        }
    }
}
