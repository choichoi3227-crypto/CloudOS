#include "pmm.h"
#include "string.h"
#include "vga.h"

#define MAX_ORDER 10 // 4KB ~ 4MB
#define BLOCK_SIZE 4096

typedef struct block {
    struct block* next;
} block_t;

static block_t* free_lists[MAX_ORDER];
static uint32_t total_blocks;

void pmm_init(uint32_t total_memory_kb) {
    total_blocks = total_memory_kb * 1024 / BLOCK_SIZE;
    for (int i = 0; i < MAX_ORDER; i++) free_lists[i] = NULL;
    
    // 사용 가능한 메모리를 가장 큰 블록 단위로 쪼개어 프리 리스트에 추가
    // (간략화: 1MB 이상 영역을 MAX_ORDER(4MB)로 채운다고 가정)
    uint32_t start_block = 256; // 1MB부터 시작
    uint32_t remaining = total_blocks - start_block;
    
    int order = MAX_ORDER - 1;
    while (remaining > 0 && order >= 0) {
        uint32_t blocks_in_order = remaining >> order;
        if (blocks_in_order > 0) {
            // 실제 물리 주소 계산하여 리스트에 연결
            for (uint32_t i = 0; i < blocks_in_order; i++) {
                block_t* b = (block_t*)((start_block + i * (1 << order)) * BLOCK_SIZE);
                b->next = free_lists[order];
                free_lists[order] = b;
            }
            remaining -= blocks_in_order << order;
            start_block += blocks_in_order << order;
        }
        order--;
    }
}

// Buddy 할당 알고리즘
uint32_t pmm_alloc_block(void) {
    // 가장 작은 블록(4KB) 요청 시
    int order = 0;
    int current_order = order;
    
    // 사용 가능한 블록을 찾을 때까지 상위 오더 탐색
    while (current_order < MAX_ORDER && !free_lists[current_order]) {
        current_order++;
    }
    
    if (current_order == MAX_ORDER) return 0; // 메모리 부족
    
    // 블록을 반으로 쪼개며 하위 오더로 이동
    while (current_order > order) {
        block_t* b = free_lists[current_order];
        free_lists[current_order] = b->next;
        
        current_order--;
        block_t* buddy = (block_t*)((uint32_t)b + (1 << current_order) * BLOCK_SIZE);
        buddy->next = free_lists[current_order];
        free_lists[current_order] = buddy;
        free_lists[current_order] = b; // 원본 블록을 하위 오더로 이동
    }
    
    // 최종 4KB 블록 반환
    block_t* b = free_lists[order];
    free_lists[order] = b->next;
    return (uint32_t)b;
}

// Buddy 해제 알고리즘 (병합 포함)
void pmm_free_block(uint32_t addr) {
    // 실제 구현에서는 주소를 기반으로 Buddy를 찾아 병합 수행
    // 여기서는 4KB 블록을 프리 리스트에 반환하는 것으로 대체
    block_t* b = (block_t*)addr;
    b->next = free_lists[0];
    free_lists[0] = b;
}
