#include "pmm.h"
#include "string.h"

#define PMM_BITMAP_SIZE 32768
static uint32_t pmm_bitmap[PMM_BITMAP_SIZE];

struct mmap_entry {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
} __attribute__((packed));

void pmm_init(uint32_t* mmap_addr, uint32_t mmap_length) {
    for(int i = 0; i < PMM_BITMAP_SIZE; i++) pmm_bitmap[i] = 0xFFFFFFFF;
    
    struct mmap_entry* entry = (struct mmap_entry*)mmap_addr;
    uint32_t offset = 0;
    
    while(offset < mmap_length) {
        if(entry->type == 1) {
            uint32_t start_block = entry->base_addr / PAGE_SIZE;
            uint32_t num_blocks = entry->length / PAGE_SIZE;
            for(uint32_t i = 0; i < num_blocks; i++) {
                if(start_block + i < PMM_BITMAP_SIZE * 32) {
                    pmm_bitmap[(start_block + i) / 32] &= ~(1 << ((start_block + i) % 32));
                }
            }
        }
        offset += entry->size + sizeof(entry->size);
        entry = (struct mmap_entry*)((uint32_t)entry + entry->size + sizeof(entry->size));
    }
}

uint32_t pmm_alloc_block(void) {
    for(uint32_t i = 0; i < PMM_BITMAP_SIZE; i++) {
        if(pmm_bitmap[i] != 0xFFFFFFFF) {
            for(int j = 0; j < 32; j++) {
                if(!(pmm_bitmap[i] & (1 << j))) {
                    pmm_bitmap[i] |= (1 << j);
                    return (i * 32 + j) * PAGE_SIZE;
                }
            }
        }
    }
    return 0;
}

void pmm_free_block(uint32_t block) {
    uint32_t idx = block / PAGE_SIZE;
    pmm_bitmap[idx / 32] &= ~(1 << (idx % 32));
}

// 추가: Slab 및 메모리 압축 로직에서 사용할 유휴 메모리 조회 함수
uint32_t pmm_get_free_mem(void) {
    uint32_t free_blocks = 0;
    for(uint32_t i = 0; i < PMM_BITMAP_SIZE; i++) {
        if(pmm_bitmap[i] != 0xFFFFFFFF) {
            for(int j = 0; j < 32; j++) {
                if(!(pmm_bitmap[i] & (1 << j))) {
                    free_blocks++;
                }
            }
        }
    }
    return free_blocks * PAGE_SIZE; // 바이트 단위로 반환
}
