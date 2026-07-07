// mouse.h
#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"

// PS/2 마우스 하드웨어 초기화 및 IRQ 활성화
void mouse_init(void);

// 인터럽트 핸들러 (IDT에서 IRQ12에 연결해야 함)
void mouse_irq_handler(void);

#endif
