#ifndef CLOUDFS_CORE_H
#define CLOUDFS_CORE_H
#include "types.h"
typedef struct { uint32_t magic; uint32_t inode_table_sector; uint32_t data_start_sector; } cloudfs_superblock_t;
typedef struct { char name[32]; uint32_t size; uint32_t start_sector; uint8_t used; } cloudfs_inode_t;
void cloudfs_init(void);
int cloudfs_write_file(const char* name, void* buf, uint32_t size);
int cloudfs_read_file(const char* name, void* buf, uint32_t size);
void cloudfs_list_files(void);
#endif
