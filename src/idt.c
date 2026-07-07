// idt.c
#include "idt.h"
#include "types.h"
#include "io.h"
#include "vga.h"

// 인터럽트 설명자 구조체 (x86 표준)
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr idtp;

// 어셈블리 파일(interrupt.asm)에 있는 스텁 함수들
extern void isr_stub_33();
extern void isr_stub_44();

// IDT 엔트리 설정 함수
static void idt_set_gate(int num, uint32_t handler, uint16_t selector, uint8_t flags) {
    idt[num].base_low = (handler & 0xFFFF);
    idt[num].base_high = (handler >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    // flags: 0x8E = 인터럽트 게이트, 커널 모드(DPL=0), 항상 활성화(P=1)
    idt[num].flags = flags;
}

// PIC(프로그래밍 가능한 인터럽트 컨트롤러) 초기화 및 리매핑
static void pic_init(void) {
    // 마스터 PIC(IRQ 0~7)을 인터럽트 32~39로 리매핑
    outb(0x20, 0x11); // ICW1: 초기화 시작 + ICW4 필요
    outb(0x21, 0x20); // ICW2: 마스터 PIC 오프셋을 32(0x20)으로 설정
    outb(0x21, 0x04); // ICW3: 슬레이브 PIC가 IRQ2에 연결됨
    outb(0x21, 0x01); // ICW4: 8086 모드

    // 슬레이브 PIC(IRQ 8~15)를 인터럽트 40~47로 리매핑
    outb(0xA0, 0x11);
    outb(0xA1, 0x40); // ICW2: 슬레이브 PIC 오프셋을 40(0x40)으로 설정
    outb(0xA1, 0x02); // ICW3: 슬레이브 식별자를 2로 설정 (마스터의 IRQ2)
    outb(0xA1, 0x01); // ICW4: 8086 모드

    // 모든 인터럽트 마스크 해제 (0x00 = 모두 허용)
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
}

void idt_init(void) {
    vga_print("[IDT] Initializing PIC and IDT...\n");
    
    // 1. PIC 리매핑 (기존 예외 인터럽트 0~31과 겹치지 않게 밀어냄)
    pic_init();

    // 2. IDT 포인터 설정
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    // 3. 키보드(IRQ 1 -> 인터럽트 33) 매핑
    idt_set_gate(33, (uint32_t)isr_stub_33, 0x08, 0x8E);
    
    // 4. 마우스(IRQ 12 -> 인터럽트 44) 매핑
    idt_set_gate(44, (uint32_t)isr_stub_44, 0x08, 0x8E);

    // 5. CPU에 IDT 테이블 위치 알림
    asm volatile("lidt (%0)" : : "r"(&idtp));
    
    vga_print("[IDT] Hardware Interrupts Ready.\n");
}
