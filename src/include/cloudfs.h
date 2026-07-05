#ifndef CLOUDFS_H
#define CLOUDFS_H
#include "types.h"

#define CLOUDFS_MAGIC 0x434C4453 // "CLDS"
#define SECTOR_SIZE 512
#define INODE_SIZE 64
#define MAX_INODES 64

typedef struct {
    uint32_t magic;
    uint32_t total_sectors;
    uint32_t inode_bitmap_sector;
    uint32_t data_bitmap_sector;
    uint32_t inode_table_sector;
    uint32_t data_start_sector;
} cloudfs_superblock_t;

typedef struct {
    char name[32];
    uint32_t size;
    uint32_t start_sector;
    uint8_t type;   // 0: Empty, 1: File, 2: Directory
    uint8_t used;
} cloudfs_inode_t;

void cloudfs_init(void);
int cloudfs_write_file(const char* name, void* buf, uint32_t size);
int cloudfs_read_file(const char* name, void* buf, uint32_t size);
void cloudfs_list_files(void);

#endif
