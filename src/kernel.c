#include "vga.h"
#include "idt.h"
#include "keyboard.h"
#include "string.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "task.h"
#include "timer.h"

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

void task1_func() {
    int i = 0;
    while(1) {
        if(i % 10000000 == 0) {
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_print("Task 1 Running...\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
        i++;
    }
}

void task2_func() {
    int i = 0;
    while(1) {
        if(i % 10000000 == 0) {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_print("Task 2 Running...\n");
            vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
        i++;
    }
}

void kernel_main(struct multiboot_info* mb_info) {
    vga_init();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("CloudOS Kernel v2.0 (Core Upgrade)\n");
    vga_print("==================================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    vga_print("[ OK ] Initializing IDT & PIC...\n");
    idt_init();
    
    vga_print("[ OK ] Initializing Physical Memory (PMM)...\n");
    pmm_init(mb_info->mmap_addr, mb_info->mmap_length);
    
    vga_print("[ OK ] Initializing Virtual Memory (VMM)...\n");
    vmm_init();
    
    vga_print("[ OK ] Initializing Kernel Heap (kmalloc)...\n");
    heap_init();
    
    vga_print("[ OK ] Initializing Task Scheduler...\n");
    task_init();
    
    vga_print("[ OK ] Initializing Keyboard...\n");
    keyboard_init();
    
    vga_print("[ OK ] Initializing Timer (100Hz)...\n");
    timer_init(100);
    
    create_task(task1_func);
    create_task(task2_func);
    
    __asm__ __volatile__("sti"); // 인터럽트 활성화 -> 스케줄러 작동 시작
    
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("\nLogin Success! (Auto-login for test)\n\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

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
            vga_print("Commands: help, clear, mem, exit\n");
        } else if (strcmp(cmd, "clear") == 0) {
            vga_init();
        } else if (strcmp(cmd, "mem") == 0) {
            vga_print("Memory: PMM, VMM, Heap active.\n");
            void* ptr = kmalloc(128);
            vga_print("Allocated 128 bytes for test.\n");
            kfree(ptr);
            vga_print("Memory freed.\n");
        } else if (idx > 0) {
            vga_print("Unknown command: ");
            vga_print(cmd);
            vga_print("\n");
        }
    }
}
