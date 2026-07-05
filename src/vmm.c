#include "vmm.h"
#include "pmm.h"

static uint64_t* pml4;

void vmm_init(void) {
    pml4 = (uint64_t*)pmm_alloc_block();
    for(int i = 0; i < 512; i++) pml4[i] = 0;
}

void* vmm_map_page(physaddr_t phys, virtaddr_t virt, uint32_t flags) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx = (virt >> 21) & 0x1FF;
    uint64_t pt_idx = (virt >> 12) & 0x1FF;

    uint64_t* pdpt;
    if(!(pml4[pml4_idx] & PAGE_PRESENT)) {
        pdpt = (uint64_t*)pmm_alloc_block();
        for(int i=0; i<512; i++) pdpt[i] = 0;
        pml4[pml4_idx] = (uint64_t)pdpt | flags | PAGE_WRITABLE;
    } else {
        pdpt = (uint64_t*)(pml4[pml4_idx] & ~0xFFF);
    }

    uint64_t* pd;
    if(!(pdpt[pdpt_idx] & PAGE_PRESENT)) {
        pd = (uint64_t*)pmm_alloc_block();
        for(int i=0; i<512; i++) pd[i] = 0;
        pdpt[pdpt_idx] = (uint64_t)pd | flags | PAGE_WRITABLE;
    } else {
        pd = (uint64_t*)(pdpt[pdpt_idx] & ~0xFFF);
    }

    uint64_t* pt;
    if(!(pd[pd_idx] & PAGE_PRESENT)) {
        pt = (uint64_t*)pmm_alloc_block();
        for(int i=0; i<512; i++) pt[i] = 0;
        pd[pd_idx] = (uint64_t)pt | flags | PAGE_WRITABLE;
    } else {
        pt = (uint64_t*)(pd[pd_idx] & ~0xFFF);
    }

    pt[pt_idx] = phys | flags | PAGE_PRESENT;
    return (void*)virt;
}
