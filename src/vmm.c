#include "vmm.h"
#include "pmm.h"
#include "string.h"
#include "vga.h"

static uint64_t* pml4;

void vmm_init(void) {
    pml4 = (uint64_t*)pmm_alloc_block();
    for(int i = 0; i < 512; i++) pml4[i] = 0;

    // 0~4GB 영역을 1:1(Identity) 매핑하여 커널 및 하드웨어 MMIO 접근 허용
    for (uint64_t i = 0; i < 1024; i++) { // 1024 * 2MB = 2GB 매핑
        uint64_t virt = i * 0x200000;
        uint64_t phys = virt;
        
        uint64_t pml4_idx = (virt >> 39) & 0x1FF;
        uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
        uint64_t pd_idx = (virt >> 21) & 0x1FF;

        uint64_t* pdpt;
        if(!(pml4[pml4_idx] & 1)) {
            pdpt = (uint64_t*)pmm_alloc_block();
            memset(pdpt, 0, 4096);
            pml4[pml4_idx] = (uint64_t)pdpt | 0x3;
        } else {
            pdpt = (uint64_t*)(pml4[pml4_idx] & ~0xFFF);
        }

        uint64_t* pd;
        if(!(pdpt[pdpt_idx] & 1)) {
            pd = (uint64_t*)pmm_alloc_block();
            memset(pd, 0, 4096);
            pdpt[pdpt_idx] = (uint64_t)pd | 0x3;
        } else {
            pd = (uint64_t*)(pdpt[pdpt_idx] & ~0xFFF);
        }

        // 2MB Huge Page 설정
        pd[pd_idx] = phys | 0x83; // Present | Writable | Huge Page
    }
}

void* vmm_map_page(physaddr_t phys, virtaddr_t virt, uint32_t flags) {
    // 이제 이 함수는 물리 주소가 1:1 매핑되지 않은 새로운 영역에 사용됨
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx = (virt >> 21) & 0x1FF;
    uint64_t pt_idx = (virt >> 12) & 0x1FF;

    uint64_t* pdpt;
    if(!(pml4[pml4_idx] & 1)) {
        pdpt = (uint64_t*)pmm_alloc_block();
        memset(pdpt, 0, 4096);
        pml4[pml4_idx] = (uint64_t)pdpt | flags | 0x2;
    } else {
        pdpt = (uint64_t*)(pml4[pml4_idx] & ~0xFFF);
    }

    uint64_t* pd;
    if(!(pdpt[pdpt_idx] & 1)) {
        pd = (uint64_t*)pmm_alloc_block();
        memset(pd, 0, 4096);
        pdpt[pdpt_idx] = (uint64_t)pd | flags | 0x2;
    } else {
        pd = (uint64_t*)(pdpt[pdpt_idx] & ~0xFFF);
    }

    uint64_t* pt;
    if(!(pd[pd_idx] & 1)) {
        pt = (uint64_t*)pmm_alloc_block();
        memset(pt, 0, 4096);
        pd[pd_idx] = (uint64_t)pt | flags | 0x2;
    } else {
        pt = (uint64_t*)(pd[pd_idx] & ~0xFFF);
    }

    pt[pt_idx] = phys | flags | 0x1;
    return (void*)virt;
}
