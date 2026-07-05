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

// Ring 3에서 실행될 사용자 앱
void user_app_main() {
    // 시스템 콜을 통해 커널에 문자열 출력 요청 (int 0x80)
    const char* msg = "[User App] Hello from Ring 3!\n";
    __asm__ __volatile__(
        "int 0x80"
        : 
        : "a"(0), "D"(msg) // rax=0 (sys_write), rdi=msg
    );
    
    // 시스템 콜을 통해 파일 저장 요청
    const char* filename = "test.txt";
    const char* content = "CloudOS File System Test!";
    __asm__ __volatile__(
        "int 0x80"
        : 
        : "a"(2), "D"(filename), "S"(content), "d"(25) // rax=2 (sys_file_write)
    );
    
    // 시스템 콜을 통해 프로세스 종료 요청
    __asm__ __volatile__(
        "int 0x80"
        : 
        : "a"(1) // rax=1 (sys_exit)
    );
}

void kernel_main(struct multiboot_info* mb_info) {
    vga_init();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_print("CloudOS Kernel v3.0 (Ring 3 & VFS)\n");
    vga_print("==================================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    vga_print("[ OK ] Initializing IDT, GDT, TSS...\n");
    idt_init();
    
    vga_print("[ OK ] Initializing Memory (PMM/VMM/Heap)...\n");
    pmm_init(mb_info->mmap_addr, mb_info->mmap_length);
    vmm_init();
    heap_init();
    
    vga_print("[ OK ] Initializing VFS (Ramdisk)...\n");
    vfs_init();
    
    vga_print("[ OK ] Initializing Task Scheduler...\n");
    task_init();
    
    vga_print("[ OK ] Creating User Task (Ring 3)...\n");
    create_user_task(user_app_main);
    
    vga_print("[ OK ] Initializing Timer...\n");
    timer_init(100);
    
    __asm__ __volatile__("sti"); // 인터럽트 활성화 -> 스케줄링 시작
    
    // 커널 메인 루프
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
