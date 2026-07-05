#ifndef VMM_H
#define VMM_H
#include "types.h"
#include "pmm.h"

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2

void vmm_init(void);
void* vmm_map_page(physaddr_t phys, virtaddr_t virt, uint32_t flags);

#endif
