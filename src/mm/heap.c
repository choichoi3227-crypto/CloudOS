// src/mm/heap.c
#include "../include/types.h"

#define HEAP_SIZE 0x100000 // 1MB 커널 힙
static uint8_t heap_memory[HEAP_SIZE];
static uint32_t heap_offset = 0;

// 상용 OS 기본 메모리 할당 (간단한 Bump Allocator)
void* kmalloc(size_t size) {
    if (heap_offset + size > HEAP_SIZE) return NULL; // OOM 방지
    void* ptr = &heap_memory[heap_offset];
    heap_offset += size;
    // 16바이트 정렬 (상용 OS 표준)
    heap_offset = (heap_offset + 15) & ~15;
    return ptr;
}

void kfree(void* ptr) {
    (void)ptr; // Bump Allocator는 해제 불가, 실제 OS는 Slab Allocator 사용
}
