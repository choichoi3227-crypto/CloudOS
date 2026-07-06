#ifndef PMM_H
#define PMM_H
#include "types.h"

// 인자를 Multiboot2 정보 구조체 포인터(void*)로 변경
void pmm_init(void* mb_info);
uint32_t pmm_alloc_block(void);

#endif
