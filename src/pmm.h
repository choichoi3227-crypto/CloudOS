// src/pmm.h (기존 파일 덮어쓰기)
#ifndef PMM_H
#define PMM_H

#include "types.h"

// 물리 메모리 페이지(4KB) 할당 및 해제
// 반환값은 실제 물리 메모리 주소 (uint32_t)
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t phys_addr);

// 남은 메모리总量 반환 (KB 단위)
uint32_t pmm_get_free_memory(void);

#endif // PMM_H
