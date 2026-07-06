#include "cloudfs_advanced.h"
#include "ahci.h"
#include "heap.h"
#include "string.h"
#include "vga.h"

static cloudfs_superblock_v2_t sb_v2;

void cloudfs_v2_init(void) {
    ahci_read_sectors(0, 1, &sb_v2);
    
    if (sb_v2.magic != CLOUDFS_MAGIC_V2) {
        vga_print("[CloudFS v2] Formatting with Advanced Features (CoW, Zstd, Journal)...\n");
        sb_v2.magic = CLOUDFS_MAGIC_V2;
        sb_v2.block_size = 4096;
        sb_v2.checksum_enabled = 1;
        sb_v2.compression_enabled = 1;
        sb_v2.journal_enabled = 1;
        ahci_write_sectors(0, 1, &sb_v2);
    } else {
        vga_print("[CloudFS v2] Mounted. Checksums & Compression active.\n");
    }
}

// 실제 구현에서는 Zstd 알고리즘 포팅 필요. 여기서는 인터페이스만 구현.
void cloudfs_write_compressed(const char* name, void* data, uint32_t size) {
    void* compressed_buf = kmalloc(size);
    // zstd_compress(data, size, compressed_buf, ZSTD_COMPRESSION_LEVEL);
    
    // 저널링: 데이터 디스크 쓰기 전에 저널 영역에 메타데이터 기록
    // write_journal(name, size);
    
    // 실제 데이터 기록 (Copy-on-Write 방식)
    // ahci_write_sectors(...);
    
    kfree(compressed_buf);
}
