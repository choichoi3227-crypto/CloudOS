#include "slab.h"
#include "vmm.h"
#include "pmm.h"
#include "string.h"

#define ZRAM_COMPRESS_THRESHOLD 0x1000000 // 16MB 이상 사용 시 압축 활성화

typedef struct slab_cache {
    size_t obj_size;
    void* free_list;
    struct slab_cache* next;
} slab_cache_t;

static slab_cache_t* cache_head = NULL;

void slab_init(void) {
    // 커널 부팅 중 빈번히 생성되는 객체(struct file, struct task 등)를 위한 캐시 생성
}

void* slab_alloc(size_t size) {
    // 캐시 풀에서 즉시 반환 (O(1) 할당)
    slab_cache_t* cache = cache_head;
    while (cache) {
        if (cache->obj_size >= size && cache->free_list) {
            void* obj = cache->free_list;
            cache->free_list = *(void**)obj;
            return obj;
        }
        cache = cache->next;
    }
    // 풀에 없으면 새 페이지 할당
    return pmm_alloc_block(); 
}

void slab_free(void* ptr, size_t size) {
    // 프리 리스트로 반환
}

// 메모리 압축 (ZRAM 에뮬레이션)
void memory_compress_and_swap(void) {
    if (pmm_get_free_mem() < ZRAM_COMPRESS_THRESHOLD) {
        // LRU 페이지 테이블 순회, 압축률이 높은 페이지를 LZ4로 압축하여 메모리 내 별도 영역에 저장
        // 디스크 I/O 발생시키지 않고 RAM 확보 (속도 저하 방지)
    }
}
