#ifndef IDT_H
#define IDT_H
#include "types.h"
struct registers { uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rdx, rcx, rbx, rax, int_no, err_code, rip, cs, rflags, rsp, ss; };
struct idt_entry { uint16_t base_low; uint16_t selector; uint8_t ist; uint8_t flags; uint16_t base_mid; uint32_t base_high; uint32_t zero; } __attribute__((packed));
struct idt_ptr { uint16_t limit; uint64_t base; } __attribute__((packed));
void idt_init(void);
extern void asm_irq_handler();
#endif
