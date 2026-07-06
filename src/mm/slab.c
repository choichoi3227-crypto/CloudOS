#include "slab.h"
#include "pmm.h"
#include "string.h"

#define SLAB_SIZE 4096
#define MAX_CACHES 16

typedef struct slab_cache {
    size_t obj_size;
    void* free_list;
    int active;
} slab_cache_t;

static slab_cache_t caches[MAX_CACHES];

void slab_init(void) {
    for (int i = 0; i < MAX_CACHES; i++) caches[i].active = 0;
}

// 새로운 캐시 생성 (예: 파일 디스크립터용 128바이트 캐시)
int slab_create_cache(size_t size) {
    for (int i = 0; i < MAX_CACHES; i++) {
        if (!caches[i].active) {
            caches[i].obj_size = size;
            caches[i].free_list = NULL;
            caches[i].active = 1;
            return i;
        }
    }
    return -1;
}

void* slab_alloc(size_t size) {
    // 적절한 크기의 캐시 찾기
    for (int i = 0; i < MAX_CACHES; i++) {
        if (caches[i].active && caches[i].obj_size >= size) {
            if (!caches[i].free_list) {
                // 프리 리스트가 비었으면 PMM에서 4KB 페이지 할당받아 객체들로 채움
                uint32_t page = pmm_alloc_block();
                if (!page) return NULL;
                
                int num_objs = SLAB_SIZE / caches[i].obj_size;
                for (int j = 0; j < num_objs; j++) {
                    void* obj = (void*)(page + j * caches[i].obj_size);
                    *(void**)obj = caches[i].free_list;
                    caches[i].free_list = obj;
                }
            }
            // 프리 리스트에서 객체 하나 반환
            void* obj = caches[i].free_list;
            caches[i].free_list = *(void**)obj;
            return obj;
        }
    }
    return NULL; // 적절한 캐시 없음
}

void slab_free(void* ptr, size_t size) {
    // 실제 구현에서는 ptr이 속한 캐시를 찾아 프리 리스트로 반환
    // 여기서는 첫 번째 캐시로 가정
    if (caches[0].active) {
        *(void**)ptr = caches[0].free_list;
        caches[0].free_list = ptr;
    }
}
