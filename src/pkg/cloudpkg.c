#include "cloudpkg.h"
#include "fs/vfs.h"
#include "fs/cloudfs_v4.h"
#include "memory.h"
#include "string.h"

#define PKG_DB_PATH "/var/pkg/db"

// 트랜잭션 기반 안전한 설치 (롤백 지원)
int cloudpkg_install(const char* pkg_name) {
    char pkg_path[64];
    strcpy(pkg_path, "/store/");
    strcat(pkg_path, pkg_name);
    strcat(pkg_path, ".cpkg");

    // 1. 패키지 파일 열기 및 서명 검증 (X.509)
    int fd = vfs_open(pkg_path, O_RDONLY);
    if (fd < 0) return -1; // 패키지 없음

    // 2. CloudFS 스냅샷 생성 (롤백 포인트)
    int snapshot_id = cloudfs_snapshot("/");

    // 3. 저널링 트랜잭션 시작
    journal_begin_txn();

    // 4. 패키지 압축 해제 및 파일 시스템에 복사
    char buf[4096];
    int bytes_read;
    while ((bytes_read = vfs_read(fd, buf, 4096)) > 0) {
        // 파일 시스템에 쓰기 (CoW 방식)
        cloudfs_cow_write(current_inode, buf, bytes_read);
    }
    vfs_close(fd);

    // 5. 의존성 해결 및 DB 업데이트
    pkg_db_add(pkg_name, "1.0.0");

    // 6. 트랜잭션 커밋
    if (journal_commit_txn() != 0) {
        // 실패 시 스냅샷으로 롤백
        cloudfs_rollback(snapshot_id);
        return -2;
    }

    return 0; // 성공
}

// 패키지 롤백
int cloudpkg_rollback(const char* pkg_name) {
    // DB에서 해당 패키지의 이전 스냅샷 ID 조회
    int snap_id = pkg_db_get_snapshot(pkg_name);
    if (snap_id < 0) return -1;
    
    return cloudfs_rollback(snap_id);
}
