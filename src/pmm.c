// src/pmm.c (기존 파일 덮어쓰기)
#include "pmm.h"
#include "vga.h"

#define PAGE_SIZE 4096
#define MAX_PAGES 65536 // 최대 256MB 물리 메모리 관리 가정

static uint32_t physical_memory_bitmap[MAX_PAGES / 32];
static uint32_t total_memory_kb = 0;

void pmm_init(uint32_t mem_kb) {
    total_memory_kb = mem_kb;
    uint32_t total_pages = (mem_kb * 1024) / PAGE_SIZE;
    
    // 비트맵 초기화 (사용 가능한 메모리는 0으로 설정)
    for (uint32_t i = 0; i < MAX_PAGES / 32; i++) {
        physical_memory_bitmap[i] = 0xFFFFFFFF; // 일단 전부 할당됨으로 마킹
    }
    
    // 커널이 사용하는 영역(약 1MB)은 할당된 것으로 유지하고, 나머지는 해제
    for (uint32_t i = 256; i < total_pages && i < MAX_PAGES; i++) { // 256페이지 = 1MB
        uint32_t idx = i / 32;
        uint32_t bit = i % 32;
        physical_memory_bitmap[idx] &= ~(1 << bit);
    }
}

uint32_t pmm_alloc_page(void) {
    for (uint32_t i = 0; i < MAX_PAGES; i++) {
        uint32_t idx = i / 32;
        uint32_t bit = i % 32;
        if (!(physical_memory_bitmap[idx] & (1 << bit))) {
            physical_memory_bitmap[idx] |= (1 << bit); // 할당 마킹
            return i * PAGE_SIZE;
        }
    }
    return 0; // Out of Memory
}

void pmm_free_page(uint32_t phys_addr) {
    if (phys_addr == 0) return;
    uint32_t page_idx = phys_addr / PAGE_SIZE;
    if (page_idx >= MAX_PAGES) return;
    
    uint32_t idx = page_idx / 32;
    uint32_t bit = page_idx % 32;
    physical_memory_bitmap[idx] &= ~(1 << bit); // 해제 마킹
}

uint32_t pmm_get_free_memory(void) {
    uint32_t free_pages = 0;
    for (uint32_t i = 0; i < MAX_PAGES; i++) {
        uint32_t idx = i / 32;
        uint32_t bit = i % 32;
        if (!(physical_memory_bitmap[idx] & (1 << bit))) {
            free_pages++;
        }
    }
    return (free_pages * PAGE_SIZE) / 1024;
}
