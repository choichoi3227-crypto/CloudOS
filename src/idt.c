#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "mouse.h"
#include "timer.h"
#include "task.h"
#include "vga.h"
#include "vfs.h"
#include "e1000.h"

struct idt_entry idt[256];
struct idt_ptr idt_ptr;

struct tss_entry {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed));

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
    uint8_t base_higher;
    uint32_t zero;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

__attribute__((aligned(16)))
static struct gdt_entry gdt[6];
static struct gdt_ptr gdt_ptr;
static struct tss_entry tss;

extern void gdt_load_tss(uint64_t gdt_ptr_addr);

static void gdt_set_entry(int i, uint16_t limit, uint64_t base, uint8_t access, uint8_t gran) {
    gdt[i].limit_low = limit & 0xFFFF;
    gdt[i].base_low = base & 0xFFFF;
    gdt[i].base_middle = (base >> 16) & 0xFF;
    gdt[i].access = access;
    gdt[i].granularity = gran;
    gdt[i].base_high = (base >> 24) & 0xFF;
    gdt[i].base_higher = (base >> 32) & 0xFFFFFFFF;
    gdt[i].zero = 0;
}

void idt_init(void) {
    gdt_set_entry(0, 0, 0, 0, 0); 
    gdt_set_entry(1, 0, 0, 0x9A, 0x20); 
    gdt_set_entry(2, 0, 0, 0x92, 0x00); 
    gdt_set_entry(3, 0, 0, 0xFA, 0x20); 
    gdt_set_entry(4, 0, 0, 0xF2, 0x00); 
    
    uint64_t tss_base = (uint64_t)&tss;
    gdt_set_entry(5, sizeof(tss), tss_base, 0x89, 0x00);
    
    extern uint8_t stack_top;
    tss.rsp0 = (uint64_t)&stack_top;
    
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint64_t)&gdt;
    gdt_load_tss((uint64_t)&gdt_ptr);

    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_ptr.base = (uint64_t)&idt;
    for (int i = 0; i < 256; i++) idt_set_gate(i, (uint64_t)asm_irq_handler);
    
    outb(0x20, 0x11); io_wait(); outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait(); outb(0xA1, 0x28); io_wait();
    outb(0x21, 0x04); io_wait(); outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait(); outb(0xA1, 0x01); io_wait();
    outb(0x21, 0x00); io_wait(); outb(0xA1, 0x00); io_wait();
    
    __asm__ __volatile__("lidt %0" : : "m"(idt_ptr));
}

void syscall_handler_c(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    switch(syscall_num) {
        case 0:
            vga_print((const char*)arg1);
            break;
        case 1:
            vga_print("\n[Syscall] User process exited.\n");
            while(1) { __asm__ __volatile__("hlt"); }
            break;
        case 2:
            vfs_write_file((const char*)arg1, (void*)arg2, arg3);
            break;
        case 3:
            vfs_read_file((const char*)arg1, (void*)arg2, arg3);
            break;
    }
}

void irq_handler_c(struct registers* regs) {
    if (regs->int_no == 0x80) {
        syscall_handler_c(regs->rax, regs->rdi, regs->rsi, regs->rdx);
    } else if (regs->int_no == 32) {
        schedule(regs);
        timer_handler();
        outb(0x20, 0x20);
    } else if (regs->int_no == 33) {
        keyboard_handle_scancode(inb(0x60));
        outb(0x20, 0x20);
    } else if (regs->int_no == 44) { // IRQ 12 (Mouse)
        mouse_handler();
        outb(0x20, 0x20);
        outb(0xA0, 0x20);
    } else if (regs->int_no == 43) {
        e1000_handler();
        outb(0x20, 0x20);
        outb(0xA0, 0x20);
    } else {
        outb(0x20, 0x20);
    }
}
