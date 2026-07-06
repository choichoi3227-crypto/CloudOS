#include "syscall.h"
#include "task.h"
#include "fs/vfs.h"
#include "net/tcp.h"
#include "memory.h"
#include "string.h"

// 시스템 콜 디스패처 (int 0x80 핸들러에서 호출)
uint64_t syscall_dispatch(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
    switch (num) {
        case SYS_FORK: {
            // 1. 현재 프로세스의 페이지 테이블(PML4) 복사 (Copy-on-Write)
            uint64_t new_pml4 = vmm_clone_user_pml4(current_task->pml4);
            // 2. 새 태스크 생성 및 레지스터 상태 복사
            int new_pid = task_create_fork(new_pml4);
            return new_pid;
        }
        case SYS_EXEC: {
            // 1. ELF 파일 로드
            uint64_t entry_point = elf_load_executable((const char*)arg1, current_task->pml4);
            if (entry_point == 0) return -1;
            // 2. 스택 초기화 및 RIP 변경
            task_replace_image(current_task, entry_point, (char**)arg2);
            return 0;
        }
        case SYS_OPEN: {
            return vfs_open((const char*)arg1, (int)arg2);
        }
        case SYS_READ: {
            return vfs_read((int)arg1, (void*)arg2, (size_t)arg3);
        }
        case SYS_WRITE: {
            return vfs_write((int)arg1, (const void*)arg2, (size_t)arg3);
        }
        case SYS_SOCKET: {
            return tcp_socket_create();
        }
        case SYS_SEND: {
            // tcp_send_data((int)arg1, (void*)arg2, (size_t)arg3);
            return arg3;
        }
        case SYS_RECV: {
            // return tcp_recv_data((int)arg1, (void*)arg2, (size_t)arg3);
            return 0;
        }
        default:
            return -1;
    }
}
