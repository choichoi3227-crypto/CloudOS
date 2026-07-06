#include "cloudfs_v4.h"
#include "ahci.h"
#include "string.h"

#define JOURNAL_START_SECTOR 1024

typedef struct {
    uint32_t magic;
    uint32_t block_size;
    uint32_t journal_size;
} cloudfs_v4_sb_t;

static cloudfs_v4_sb_t sb;

void cloudfs_v4_init(void) {
    ahci_read_sectors(0, 1, &sb);
    if (sb.magic != 0xC1044F57) { // "COW"
        sb.magic = 0xC1044O57;
        sb.block_size = 4096;
        sb.journal_size = 1024;
        ahci_write_sectors(0, 1, &sb);
    }
}

// Copy-on-Write 쓰기 로직
int cloudfs_cow_write(int inode, const void* buf, uint32_t size) {
    // 1. 저널에 트랜잭션 시작 기록
    // 2. 새로운 빈 데이터 블록 할당
    // 3. 데이터를 Zstd 알고리즘으로 압축하여 새 블록에 쓰기
    // 4. 체크섬(CRC32c) 계산하여 메타데이터에 기록
    // 5. 아이노드의 블록 포인터를 새 블록으로 업데이트 (원자적 연산)
    // 6. 저널에 트랜잭션 커밋 기록
    return size;
}
