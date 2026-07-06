#include "vmm.h"
#include "pmm.h"
#include "string.h"

static uint64_t* kernel_pml4;

void vmm_init(void) {
    kernel_pml4 = (uint64_t*)pmm_alloc_block();
    for(int i = 0; i < 512; i++) kernel_pml4[i] = 0;

    for (uint64_t i = 0; i < 1024; i++) {
        uint64_t virt = i * 0x200000;
        uint64_t phys = virt;
        
        uint64_t pml4_idx = (virt >> 39) & 0x1FF;
        uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
        uint64_t pd_idx = (virt >> 21) & 0x1FF;

        uint64_t* pdpt;
        if(!(kernel_pml4[pml4_idx] & 1)) {
            pdpt = (uint64_t*)pmm_alloc_block();
            memset(pdpt, 0, 4096);
            kernel_pml4[pml4_idx] = (uint64_t)pdpt | 0x3;
        } else {
            pdpt = (uint64_t*)(kernel_pml4[pml4_idx] & ~0xFFF);
        }

        uint64_t* pd;
        if(!(pdpt[pdpt_idx] & 1)) {
            pd = (uint64_t*)pmm_alloc_block();
            memset(pd, 0, 4096);
            pdpt[pdpt_idx] = (uint64_t)pd | 0x3;
        } else {
            pd = (uint64_t*)(pdpt[pdpt_idx] & ~0xFFF);
        }

        pd[pd_idx] = phys | 0x83; // Huge Page
    }
}

static uint64_t get_kernel_pml4() {
    return (uint64_t)kernel_pml4;
}

uint64_t vmm_create_user_pml4(void) {
    uint64_t* new_pml4 = (uint64_t*)pmm_alloc_block();
    memset(new_pml4, 0, 4096);
    for(int i = 256; i < 512; i++) {
        new_pml4[i] = kernel_pml4[i];
    }
    return (uint64_t)new_pml4;
}

// 수정: executable 인자 추가 및 NX 비트 처리
void vmm_map_page_to_pml4(uint64_t pml4_addr, physaddr_t phys, virtaddr_t virt, uint32_t flags, int executable) {
    uint64_t* pml4 = (uint64_t*)pml4_addr;
    
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

    uint64_t entry_flags = phys | flags | 0x1;
    if (!executable) {
        entry_flags |= PAGE_NX; // 실행 불가능 영역 설정 (DEP)
    }
    pt[pt_idx] = entry_flags;
}

void* vmm_map_page(physaddr_t phys, virtaddr_t virt, uint32_t flags) {
    // 커널 매핑은 기본적으로 실행 가능으로 처리
    vmm_map_page_to_pml4(get_kernel_pml4(), phys, virt, flags, 1);
    return (void*)virt;
}
