#include "pmm.h"
#include "string.h"

#define PMM_BITMAP_SIZE 32768
static uint32_t pmm_bitmap[PMM_BITMAP_SIZE];

// Multiboot2 태그 헤더
struct multiboot_tag {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
} __attribute__((packed));

// Multiboot2 메모리 맵 엔트리
struct multiboot_mmap_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

// Multiboot2 메모리 맵 태그
struct multiboot_tag_mmap {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry entries[];
} __attribute__((packed));

void pmm_init(void* mb_info) {
    // 1. 비트맵 초기화 (모두 사용 중으로 설정)
    for(int i = 0; i < PMM_BITMAP_SIZE; i++) pmm_bitmap[i] = 0xFFFFFFFF;
    
    // 2. Multiboot2 정보 파싱 시작
    uint32_t* info = (uint32_t*)mb_info;
    uint32_t total_size = info[0];
    
    // 첫 번째 태그는 헤더(8바이트) 다음부터 시작
    uint8_t* ptr = (uint8_t*)(info + 2); 
    uint8_t* end = (uint8_t*)info + total_size;

    // 3. 태그 순회
    while (ptr < end) {
        struct multiboot_tag* tag = (struct multiboot_tag*)ptr;
        
        // 태그 타입이 0이면 끝
        if (tag->type == 0) break;
        
        // 태그 타입이 6이면 메모리 맵(MMAP)
        if (tag->type == 6) {
            struct multiboot_tag_mmap* mmap_tag = (struct multiboot_tag_mmap*)ptr;
            uint32_t entry_size = mmap_tag->entry_size;
            
            uint8_t* entry_ptr = (uint8_t*)mmap_tag->entries;
            uint8_t* entries_end = ptr + tag->size;
            
            // 4. 메모리 맵 엔트리 순회하며 사용 가능한 RAM 마킹
            while (entry_ptr < entries_end) {
                struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*)entry_ptr;
                if (entry->type == 1) { // Type 1 = Available RAM
                    uint32_t start_block = entry->base_addr / PAGE_SIZE;
                    uint32_t num_blocks = entry->length / PAGE_SIZE;
                    
                    for(uint32_t i = 0; i < num_blocks; i++) {
                        if(start_block + i < PMM_BITMAP_SIZE * 32) {
                            pmm_bitmap[(start_block + i) / 32] &= ~(1 << ((start_block + i) % 32));
                        }
                    }
                }
                entry_ptr += entry_size;
            }
        }
        
        // 다음 태그로 이동 (8바이트 정렬)
        ptr += (tag->size + 7) & ~7; 
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
    return 0; // Out of memory
}
