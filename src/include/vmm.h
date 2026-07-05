#ifndef VMM_H
#define VMM_H
#include "types.h"
#include "pmm.h"

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4

void vmm_init(void);
void* vmm_map_page(physaddr_t phys, virtaddr_t virt, uint32_t flags);
uint64_t vmm_create_user_pml4(void);
void vmm_map_page_to_pml4(uint64_t pml4, physaddr_t phys, virtaddr_t virt, uint32_t flags);

#endif
