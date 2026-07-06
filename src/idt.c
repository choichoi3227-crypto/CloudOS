#include <idt.h>
#include <io.h>
#include <keyboard.h>

struct idt_entry idt[256];
struct idt_ptr idt_ptr;

void idt_set_gate(int n, uint64_t handler, int is_err) {
    idt[n].base_low = handler & 0xFFFF;
    idt[n].base_mid = (handler >> 16) & 0xFFFF;
    idt[n].base_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].selector = 0x08;
    idt[n].ist = 0;
    idt[n].flags = 0x8E;
    idt[n].zero = 0;
}

extern void irq32();
extern void irq33();
extern void irq34();
extern void irq35();
extern void irq36();
extern void irq37();
extern void irq38();
extern void irq39();
extern void irq40();
extern void irq41();
extern void irq42();
extern void irq43();
extern void irq44();
extern void irq45();
extern void irq46();
extern void irq47();

void idt_init(void) {
    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_ptr.base = (uint64_t)&idt;
    
    for (int i = 0; i < 256; i++) idt_set_gate(i, 0, 0);
    
    idt_set_gate(32, (uint64_t)irq32, 0);
    idt_set_gate(33, (uint64_t)irq33, 0);
    idt_set_gate(34, (uint64_t)irq34, 0);
    idt_set_gate(35, (uint64_t)irq35, 0);
    idt_set_gate(36, (uint64_t)irq36, 0);
    idt_set_gate(37, (uint64_t)irq37, 0);
    idt_set_gate(38, (uint64_t)irq38, 0);
    idt_set_gate(39, (uint64_t)irq39, 0);
    idt_set_gate(40, (uint64_t)irq40, 0);
    idt_set_gate(41, (uint64_t)irq41, 0);
    idt_set_gate(42, (uint64_t)irq42, 0);
    idt_set_gate(43, (uint64_t)irq43, 0);
    idt_set_gate(44, (uint64_t)irq44, 0);
    idt_set_gate(45, (uint64_t)irq45, 0);
    idt_set_gate(46, (uint64_t)irq46, 0);
    idt_set_gate(47, (uint64_t)irq47, 0);
    
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
