#include "timer.h"
#include "io.h"

uint64_t timer_ticks = 0; // 수정: static 제거 및 전역 변수 초기화

void timer_init(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

void timer_handler(void) {
    timer_ticks++;
}
