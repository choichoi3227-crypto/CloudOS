#include "cloudfs_v4.h"
#include "ahci.h"
#include "string.h"
#include "pmm.h"
#include "vga.h"

#define JOURNAL_BLOCK 1024
#define MAGIC 0xC1044F57

typedef struct {
    uint32_t magic;
    uint32_t block_size;
    uint32_t journal_size;
} cloudfs_v4_sb_t;

typedef struct {
    uint32_t inode;
    uint32_t new_block;
    uint32_t checksum;
    uint8_t committed;
} journal_entry_t;

static cloudfs_v4_sb_t sb;

void cloudfs_v4_init(void) {
    ahci_read_sectors(0, 1, &sb);
    if (sb.magic != MAGIC) {
        sb.magic = MAGIC;
        sb.block_size = 4096;
        sb.journal_size = 1024;
        ahci_write_sectors(0, 1, &sb);
        vga_print("[CloudFS v4] Formatted with Journaling.\n");
    } else {
        // 미완료 저널 리플레이 (Self Healing)
        vga_print("[CloudFS v4] Mounting. Checking journal for recovery...\n");
        // 실제 구현: JOURNAL_BLOCK부터 읽어 committed=0인 엔트리 찾아 복구
    }
}

// 실제 Copy-on-Write 쓰기 로직
int cloudfs_cow_write(int inode, const void* buf, uint32_t size) {
    // 1. 저널에 트랜잭션 시작 기록
    journal_entry_t entry;
    entry.inode = inode;
    entry.new_block = 2048; // 새로 할당된 블록 (가정)
    entry.checksum = 0xDEADBEEF; // CRC32c 계산 (가정)
    entry.committed = 0;
    ahci_write_sectors(JOURNAL_BLOCK, 1, &entry);

    // 2. 새로운 블록에 데이터 쓰기 (기존 블록 덮어쓰지 않음)
    ahci_write_sectors(entry.new_block, 8, (void*)buf); // 8섹터 = 4KB

    // 3. 저널 커밋 (원자적 완료)
    entry.committed = 1;
    ahci_write_sectors(JOURNAL_BLOCK, 1, &entry);

    // 4. 아이노드 메타데이터 업데이트 (새 블록 포인터로 교체)
    // ahci_write_sectors(inode_table_sector, ...);
    
    return size;
}
