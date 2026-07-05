#ifndef IDT_H
#define IDT_H
#include "types.h"
struct idt_entry { uint16_t base_low; uint16_t selector; uint8_t ist; uint8_t flags; uint16_t base_mid; uint32_t base_high; uint32_t zero; } __attribute__((packed));
struct idt_ptr { uint16_t limit; uint64_t base; } __attribute__((packed));
void idt_init(void);
void irq_handler_c(uint64_t* rsp);
static inline void outb(uint16_t port, uint8_t val) { __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port)); }
static inline uint8_t inb(uint16_t port) { uint8_t ret; __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port)); return ret; }
#endif
