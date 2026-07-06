#ifndef CLOUDFS_ADVANCED_H
#define CLOUDFS_ADVANCED_H
#include "types.h"

#define CLOUDFS_MAGIC_V2 0x434C4F56 // "CLOV" (CloudFS v2)
#define ZSTD_COMPRESSION_LEVEL 3

typedef struct {
    uint32_t magic;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t checksum_enabled;
    uint32_t compression_enabled;
    uint32_t journal_enabled;
} cloudfs_superblock_v2_t;

void cloudfs_v2_init(void);
void cloudfs_write_compressed(const char* name, void* data, uint32_t size);

#endif
