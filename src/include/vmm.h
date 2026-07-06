#ifndef VMM_H
#define VMM_H
#include "types.h"
#include "pmm.h"

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER 0x4
#define PAGE_NX (1ULL << 63) // 추가: No-Execute 비트

void vmm_init(void);
void* vmm_map_page(physaddr_t phys, virtaddr_t virt, uint32_t flags);
uint64_t vmm_create_user_pml4(void);
// 수정: executable 플래그 추가 (0이면 NX 설정, 1이면 실행 가능)
void vmm_map_page_to_pml4(uint64_t pml4, physaddr_t phys, virtaddr_t virt, uint32_t flags, int executable);

#endif
