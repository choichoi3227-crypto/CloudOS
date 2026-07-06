#ifndef TIMER_H
#define TIMER_H
#include "types.h"

extern uint64_t timer_ticks; // 추가: 커널 전체에서 접근 가능하도록 선언

void timer_init(uint32_t freq);
void timer_handler(void);

#endif
