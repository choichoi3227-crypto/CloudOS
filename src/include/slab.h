#ifndef SLAB_H
#define SLAB_H
#include "types.h"

void slab_init(void);
void* slab_alloc(size_t size);
void slab_free(void* ptr, size_t size);
void memory_compress_and_swap(void);

#endif
