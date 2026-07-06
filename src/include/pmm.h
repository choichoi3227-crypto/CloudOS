#ifndef PMM_H
#define PMM_H
#include <types.h>
void pmm_init(void* mb_info);
uint32_t pmm_alloc_block(void);
#endif
