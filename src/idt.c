#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "timer.h"
#include "task.h"
#include "vga.h"
#include "vfs.h"
#include "e1000.h"

// ... (기존 idt_init, gdt 설정 동일)

void irq_handler_c(struct registers* regs) {
    if (regs->int_no == 0x80) { // System Call
        syscall_handler_c(regs->rax, regs->rdi, regs->rsi, regs->rdx);
    } else if (regs->int_no == 32) { // Timer
        schedule(regs);
        timer_handler();
        outb(0x20, 0x20);
    } else if (regs->int_no == 33) { // Keyboard
        keyboard_handle_scancode(inb(0x60));
        outb(0x20, 0x20);
    } else if (regs->int_no == 43) { // IRQ11 (Network)
        e1000_handler();
        outb(0x20, 0x20);
        outb(0xA0, 0x20); // Slave PIC EOI
    } else {
        outb(0x20, 0x20);
    }
}
