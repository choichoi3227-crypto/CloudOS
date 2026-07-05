#include "cloudfs.h"
#include "ahci.h"
#include "vga.h"
#include "string.h"
#include "heap.h"

static cloudfs_superblock_t sb;
static cloudfs_inode_t inodes[MAX_INODES];

void cloudfs_init(void) {
    // 슈퍼블록 읽기 (섹터 0)
    ahci_read_sectors(0, 1, &sb);
    
    if (sb.magic != CLOUDFS_MAGIC) {
        vga_print("[CloudFS] Invalid magic. Formatting disk...\n");
        sb.magic = CLOUDFS_MAGIC;
        sb.total_sectors = 32768; // 16MB 가정
        sb.inode_bitmap_sector = 1;
        sb.data_bitmap_sector = 2;
        sb.inode_table_sector = 3;
        sb.data_start_sector = 3 + (MAX_INODES * INODE_SIZE / SECTOR_SIZE);
        
        // 메모리 상에 아이노드 테이블 초기화 후 디스크로 기록
        memset(inodes, 0, sizeof(inodes));
        ahci_write_sectors(sb.inode_table_sector, (MAX_INODES * INODE_SIZE) / SECTOR_SIZE + 1, inodes);
        ahci_write_sectors(0, 1, &sb);
        vga_print("[CloudFS] Format complete.\n");
    } else {
        // 아이노드 테이블 로드
        ahci_read_sectors(sb.inode_table_sector, (MAX_INODES * INODE_SIZE) / SECTOR_SIZE + 1, inodes);
        vga_print("[CloudFS] Mount successful.\n");
    }
}

int cloudfs_write_file(const char* name, void* buf, uint32_t size) {
    // 빈 아이노드 찾기
    for (int i = 0; i < MAX_INODES; i++) {
        if (!inodes[i].used) {
            strcpy(inodes[i].name, name);
            inodes[i].size = size;
            inodes[i].start_sector = sb.data_start_sector + (i * 16); // 파일당 16섹터(8KB) 할당
            inodes[i].type = 1;
            inodes[i].used = 1;
            
            // 데이터 기록
            uint32_t sectors_to_write = (size + SECTOR_SIZE - 1) / SECTOR_SIZE;
            ahci_write_sectors(inodes[i].start_sector, sectors_to_write, buf);
            
            // 메타데이터(아이노드) 기록
            ahci_write_sectors(sb.inode_table_sector, (MAX_INODES * INODE_SIZE) / SECTOR_SIZE + 1, inodes);
            return size;
        }
    }
    return -1; // 공간 부족
}

int cloudfs_read_file(const char* name, void* buf, uint32_t size) {
    for (int i = 0; i < MAX_INODES; i++) {
        if (inodes[i].used && strcmp(inodes[i].name, name) == 0) {
            uint32_t sectors_to_read = (inodes[i].size + SECTOR_SIZE - 1) / SECTOR_SIZE;
            ahci_read_sectors(inodes[i].start_sector, sectors_to_read, buf);
            return inodes[i].size;
        }
    }
    return -1; // 파일 없음
}

void cloudfs_list_files(void) {
    vga_print("Name            Size\n");
    vga_print("----------------------\n");
    for (int i = 0; i < MAX_INODES; i++) {
        if (inodes[i].used) {
            vga_print(inodes[i].name);
            vga_print("            ");
            // 사이즈 출력은 간략화하기 위해 생략 (실제로는 itoa 필요)
            vga_print("\n");
        }
    }
}
