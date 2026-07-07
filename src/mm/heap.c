// src/mm/heap.c (기존 파일 덮어쓰기)
#include "mm/heap.h"

#define HEAP_SIZE 0x200000 // 2MB 커널 힙 (GUI 여유분 고려)
static uint8_t heap_memory[HEAP_SIZE];
static uint32_t heap_offset = 0;

// 상용 OS 기본 메모리 할당 (Bump Allocator)
void* kmalloc(size_t size) {
    // 16바이트 정렬 요구사항 충족
    size_t aligned_size = (size + 15) & ~15;
    
    if (heap_offset + aligned_size > HEAP_SIZE) {
        return NULL; // Out Of Memory 방지
    }
    
    void* ptr = &heap_memory[heap_offset];
    heap_offset += aligned_size;
    return ptr;
}

// 현재 단계에서는 메모리 해제 불가 (Bump Allocator 특징)
// 실제 상용 OS는 Slab Allocator를 사용하여 해제를 지원함
void kfree(void* ptr) {
    (void)ptr; 
}
