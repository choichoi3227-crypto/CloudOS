#include "heap.h"
#include "vmm.h"
#include "pmm.h"
#include "string.h"

#define HEAP_START 0x1000000 // 16MB 지점
#define HEAP_SIZE  0x100000  // 1MB 힙

struct heap_block {
    size_t size;
    int is_free;
    struct heap_block* next;
    struct heap_block* prev;
} __attribute__((packed));

static struct heap_block* heap_head;

void heap_init(void) {
    for(uint32_t i = 0; i < HEAP_SIZE; i += PAGE_SIZE) {
        vmm_map_page(pmm_alloc_block(), HEAP_START + i, PAGE_PRESENT | PAGE_WRITABLE);
    }
    heap_head = (struct heap_block*)HEAP_START;
    heap_head->size = HEAP_SIZE - sizeof(struct heap_block);
    heap_head->is_free = 1;
    heap_head->next = NULL;
    heap_head->prev = NULL;
}

void* kmalloc(size_t size) {
    struct heap_block* curr = heap_head;
    while(curr != NULL) {
        if(curr->is_free && curr->size >= size) {
            if(curr->size > size + sizeof(struct heap_block) + 16) {
                struct heap_block* new_block = (struct heap_block*)((uint8_t*)curr + sizeof(struct heap_block) + size);
                new_block->size = curr->size - size - sizeof(struct heap_block);
                new_block->is_free = 1;
                new_block->next = curr->next;
                new_block->prev = curr;
                if(curr->next) curr->next->prev = new_block;
                curr->next = new_block;
                curr->size = size;
            }
            curr->is_free = 0;
            return (void*)((uint8_t*)curr + sizeof(struct heap_block));
        }
        curr = curr->next;
    }
    return NULL;
}

void kfree(void* ptr) {
    if(!ptr) return;
    struct heap_block* block = (struct heap_block*)((uint8_t*)ptr - sizeof(struct heap_block));
    block->is_free = 1;
    if(block->next && block->next->is_free) {
        block->size += block->next->size + sizeof(struct heap_block);
        block->next = block->next->next;
        if(block->next) block->next->prev = block;
    }
}
