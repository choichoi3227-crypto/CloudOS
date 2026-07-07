// src/idt.c (기존 파일 완전 덮어쓰기)
#include "idt.h"
#include "io.h"
#include "vga.h"

// [삭제됨] 이 파일 내부에 struct idt_entry가 있었다면 지워주세요. 아래는 헤더에 의존하는 깔끔한 코드입니다.

static struct idt_entry idt[256];
static struct idt_ptr idtp;

// 어셈블리 래퍼(interrupt.asm)에서 만든 함수들
extern void isr_stub_33();
extern void isr_stub_44();

// IDT 엔트리 설정 함수
static void idt_set_gate(int num, uint32_t handler, uint16_t selector, uint8_t flags) {
    idt[num].base_low = (handler & 0xFFFF);
    idt[num].base_high = (handler >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags; // 0x8E = 인터럽트 게이트, DPL=0
}

// PIC 초기화 및 리매핑
static void pic_init(void) {
    outb(0x20, 0x11); outb(0x21, 0x20); outb(0x21, 0x04); outb(0x21, 0x01);
    outb(0xA0, 0x11); outb(0xA1, 0x40); outb(0xA1, 0x02); outb(0xA1, 0x01);
    outb(0x21, 0x00); outb(0xA1, 0x00); // 모든 인터럽트 마스크 해제
}

void idt_init(void) {
    vga_print("[IDT] Initializing PIC and IDT...\n");
    pic_init();

    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    // 키보드(IRQ 1 -> INT 33) 매핑
    idt_set_gate(33, (uint32_t)isr_stub_33, 0x08, 0x8E);
    // 마우스(IRQ 12 -> INT 44) 매핑
    idt_set_gate(44, (uint32_t)isr_stub_44, 0x08, 0x8E);

    asm volatile("lidt (%0)" : : "r"(&idtp));
    vga_print("[IDT] Hardware Interrupts Ready.\n");
}
