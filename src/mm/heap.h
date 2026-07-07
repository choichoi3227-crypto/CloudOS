// src/mm/heap.h (새 파일 생성)
#ifndef MM_HEAP_H
#define MM_HEAP_H

#include "../include/types.h"

// 커널 메모리 동적 할당 (GUI 컴포지터가 3MB 메모리를 요구할 때 사용됨)
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif // MM_HEAP_H
