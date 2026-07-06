#include "cloudfs_core.h"
#include "ide.h"
#include "string.h"
#include "vga.h"

#define CLOUDFS_MAGIC 0x434C4453
#define MAX_INODES 64

static cloudfs_superblock_t sb;
static cloudfs_inode_t inodes[MAX_INODES];

void cloudfs_init(void) {
    ide_read_sectors(0, 1, &sb);
    
    if (sb.magic != CLOUDFS_MAGIC) {
        vga_print("[CloudFS] Formatting disk...\n");
        sb.magic = CLOUDFS_MAGIC;
        sb.inode_table_sector = 1;
        sb.data_start_sector = 2 + (MAX_INODES * sizeof(cloudfs_inode_t) / 512);
        
        memset(inodes, 0, sizeof(inodes));
        ide_write_sectors(sb.inode_table_sector, 1, inodes);
        ide_write_sectors(0, 1, &sb);
    } else {
        ide_read_sectors(sb.inode_table_sector, 1, inodes);
    }
}

int cloudfs_write_file(const char* name, void* buf, uint32_t size) {
    for (int i = 0; i < MAX_INODES; i++) {
        if (!inodes[i].used) {
            strcpy(inodes[i].name, name);
            inodes[i].size = size;
            inodes[i].start_sector = sb.data_start_sector + i;
            inodes[i].used = 1;
            
            ide_write_sectors(inodes[i].start_sector, 1, buf);
            ide_write_sectors(sb.inode_table_sector, 1, inodes);
            return size;
        }
    }
    return -1;
}

int cloudfs_read_file(const char* name, void* buf, uint32_t size) {
    for (int i = 0; i < MAX_INODES; i++) {
        if (inodes[i].used && strcmp(inodes[i].name, name) == 0) {
            ide_read_sectors(inodes[i].start_sector, 1, buf);
            return inodes[i].size;
        }
    }
    return -1;
}

void cloudfs_list_files(void) {
    vga_print("Name            Size\n");
    vga_print("----------------------\n");
    for (int i = 0; i < MAX_INODES; i++) {
        if (inodes[i].used) {
            vga_print(inodes[i].name);
            vga_print("\n");
        }
    }
}
