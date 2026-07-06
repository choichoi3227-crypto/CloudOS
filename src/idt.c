#include "idt.h"
#include "io.h"
#include "keyboard.h"

struct idt_entry idt[256];
struct idt_ptr idt_ptr;

void idt_set_gate(int n, uint64_t handler) {
    idt[n].base_low = handler & 0xFFFF;
    idt[n].base_mid = (handler >> 16) & 0xFFFF;
    idt[n].base_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].selector = 0x08;
    idt[n].ist = 0;
    idt[n].flags = 0x8E;
    idt[n].zero = 0;
}

void idt_init(void) {
    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_ptr.base = (uint64_t)&idt;
    for (int i = 0; i < 256; i++) idt_set_gate(i, (uint64_t)asm_irq_handler);
    
    // PIC 리매핑
    outb(0x20, 0x11); io_wait(); outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait(); outb(0xA1, 0x28); io_wait();
    outb(0x21, 0x04); io_wait(); outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait(); outb(0xA1, 0x01); io_wait();
    outb(0x21, 0x00); io_wait(); outb(0xA1, 0x00); io_wait();
    
    __asm__ __volatile__("lidt %0" : : "m"(idt_ptr));
}

void irq_handler_c(struct registers* regs) {
    if (regs->int_no == 33) { // Keyboard
        keyboard_handle_scancode(inb(0x60));
        outb(0x20, 0x20);
    } else {
        outb(0x20, 0x20);
    }
}
