#include "idt.h"
#include "vga.h"

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

extern void asm_irq_handler();

void idt_init(void) {
    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_ptr.base = (uint64_t)&idt;
    
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint64_t)asm_irq_handler);
    }
    
    __asm__ __volatile__("lidt %0" : : "m"(idt_ptr));
    
    // PIC 리매핑 (IRQ 0-15를 인터럽트 32-47로 이동)
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
}

void irq_handler_c(uint64_t* rsp) {
    // 인터럽트 번호 확인 (스택에 푸시된 에러 코드와 인터럽트 번호 읽기)
    // 이 간략화된 구조에서는 키보드 IRQ1(벡터 33)을 가정
    uint8_t scancode = inb(0x60);
    keyboard_handle_scancode(scancode);
    
    // EOI(End of Interrupt) 전송
    outb(0x20, 0x20);
}
