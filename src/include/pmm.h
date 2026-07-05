#ifndef PMM_H
#define PMM_H
#include "types.h"

void pmm_init(uint32_t* mmap_addr, uint32_t mmap_length);
uint32_t pmm_alloc_block(void);
void pmm_free_block(uint32_t block);

#endif
