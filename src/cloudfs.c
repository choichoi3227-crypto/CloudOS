#include "cloudfs.h"
#include "ahci.h"
#include "vga.h"
#include "string.h"
#include "heap.h"

static cloudfs_superblock_t sb;
static cloudfs_inode_t inodes[MAX_INODES];

void cloudfs_init(void) {
    ahci_read_sectors(0, 1, &sb);
    
    if (sb.magic != CLOUDFS_MAGIC) {
        vga_print("[CloudFS] Invalid magic. Formatting disk...\n");
        sb.magic = CLOUDFS_MAGIC;
        sb.total_sectors = 32768;
        sb.inode_bitmap_sector = 1;
        sb.data_bitmap_sector = 2;
        sb.inode_table_sector = 3;
        sb.data_start_sector = 3 + (MAX_INODES * INODE_SIZE / SECTOR_SIZE);
        
        memset(inodes, 0, sizeof(inodes));
        ahci_write_sectors(sb.inode_table_sector, (MAX_INODES * INODE_SIZE) / SECTOR_SIZE + 1, inodes);
        ahci_write_sectors(0, 1, &sb);
        vga_print("[CloudFS] Format complete.\n");
    } else {
        ahci_read_sectors(sb.inode_table_sector, (MAX_INODES * INODE_SIZE) / SECTOR_SIZE + 1, inodes);
        vga_print("[CloudFS] Mount successful.\n");
    }
}

int cloudfs_write_file(const char* name, void* buf, uint32_t size) {
    for (int i = 0; i < MAX_INODES; i++) {
        if (!inodes[i].used) {
            strcpy(inodes[i].name, name);
            inodes[i].size = size;
            inodes[i].start_sector = sb.data_start_sector + (i * 16);
            inodes[i].type = 1;
            inodes[i].used = 1;
            
            uint32_t sectors_to_write = (size + SECTOR_SIZE - 1) / SECTOR_SIZE;
            ahci_write_sectors(inodes[i].start_sector, sectors_to_write, buf);
            
            ahci_write_sectors(sb.inode_table_sector, (MAX_INODES * INODE_SIZE) / SECTOR_SIZE + 1, inodes);
            return size;
        }
    }
    return -1;
}

int cloudfs_read_file(const char* name, void* buf, uint32_t size) {
    for (int i = 0; i < MAX_INODES; i++) {
        if (inodes[i].used && strcmp(inodes[i].name, name) == 0) {
            uint32_t sectors_to_read = (inodes[i].size + SECTOR_SIZE - 1) / SECTOR_SIZE;
            ahci_read_sectors(inodes[i].start_sector, sectors_to_read, buf);
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
            vga_print("            ");
            vga_print("\n");
        }
    }
}
