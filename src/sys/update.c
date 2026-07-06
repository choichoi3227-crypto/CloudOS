#include "update.h"
#include "fs/vfs.h"
#include "net/tcp.h"
#include "memory.h"
#include "string.h"
#include "sync.h"

static spinlock_t update_lock;

// 델타(차분) 업데이트 적용
int update_apply_delta(const char* delta_file) {
    spinlock_acquire(&update_lock);
    
    // 1. 현재 시스템 이미지의 체크섬 확인
    uint32_t current_hash = system_get_hash();
    
    // 2. 델타 파일 다운로드 (또는 로컬에서 읽기)
    int fd = vfs_open(delta_file, O_RDONLY);
    if (fd < 0) {
        spinlock_release(&update_lock);
        return -1;
    }

    // 3. A/B 파티션 스킴: 비활성 파티션에 델타 적용
    char* inactive_part = (current_hash % 2 == 0) ? "/dev/sda2" : "/dev/sda1";
    int part_fd = vfs_open(inactive_part, O_WRONLY);
    
    // 4. 바이너리 패치 적용 (BSDiff 알고리즘)
    // (구현 생략: 바이너리 블록 단위 XOR 및 교체)
    
    // 5. 부트로더(GRUB/CloudBoot)에 다음 부팅 시 적용할 파티션 지정
    bootloader_set_active(inactive_part);
    
    vfs_close(fd);
    vfs_close(part_fd);
    
    spinlock_release(&update_lock);
    return 0;
}

// 업데이트 실패 시 자동 복구
void update_verify_boot(void) {
    // 부팅 후 시스템 무결성 검증
    if (system_verify_integrity() != 0) {
        // 실패 시 이전 파티션으로 롤백
        bootloader_rollback();
        system_reboot();
    }
}
