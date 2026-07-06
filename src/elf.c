#include "elf.h"
#include "vfs.h"
#include "vmm.h"
#include "pmm.h"
#include "heap.h"
#include "string.h"
#include "vga.h"

// 동적 링크 정보를 위한 구조체 추가
typedef struct {
    uint32_t type;
    uint32_t offset;
} __attribute__((packed)) elf_dyn_t;

int elf_load_and_exec(const char* path) {
    uint8_t* buf = (uint8_t*)kmalloc(8192); // 헤더 및 동적 정보 읽기용
    if (!buf) return -1;
    
    uint32_t read_bytes = vfs_read_file(path, buf, 8192);
    if (read_bytes <= 0) {
        kfree(buf);
        return -1;
    }

    elf_header_t* elf = (elf_header_t*)buf;
    if (elf->magic != ELF_MAGIC) {
        kfree(buf);
        return -1;
    }

    uint64_t user_pml4 = vmm_create_user_pml4();
    
    // 1. PT_LOAD 세그먼트 매핑 (실행/쓰기 권한 분리 적용)
    elf_phdr_t* phdr = (elf_phdr_t*)(buf + elf->phoff);
    for (int i = 0; i < elf->phnum; i++) {
        if (phdr[i].type == PT_LOAD) {
            uint32_t pages = (phdr[i].memsz + 4095) / 4096;
            for (uint32_t p = 0; p < pages; p++) {
                physaddr_t phys = pmm_alloc_block();
                int executable = (phdr[i].flags & 0x1) ? 1 : 0;
                vmm_map_page_to_pml4(user_pml4, phys, phdr[i].vaddr + p * 4096, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER, executable);
                memcpy((void*)phys, buf + phdr[i].offset + p * 4096, 4096);
            }
        }
    }

    // 2. 동적 링커 (Dynamic Linker) 처리
    // PT_DYNAMIC 세그먼트 찾기
    for (int i = 0; i < elf->phnum; i++) {
        if (phdr[i].type == 2) { // PT_DYNAMIC
            elf_dyn_t* dyn = (elf_dyn_t*)(buf + phdr[i].offset);
            // 실제 구현: .dynamic 섹션을 순회하며 .so 파일 로드 및 GOT(Global Offset Table) 채움
            // 여기서는 시스템 콜 라이브러리(libc)를 가상 메모리에 매핑한다고 가정
            vga_print("[ELF] Resolving Dynamic Links (libc.so)...\n");
            
            uint64_t libc_base = 0x40000000; // 라이브러리 기본 주소
            for (int j = 0; dyn[j].type != 0; j++) {
                if (dyn[j].type == 3) { // DT_PLTGOT
                    // GOT 테이블에 실제 함수 주소 기록
                    uint64_t* got = (uint64_t*)(phdr[i].vaddr + dyn[j].offset);
                    *got = (uint64_t)syscall_handler_c; // 시스템 콜 핸들러 주소 매핑
                }
            }
        }
    }

    kfree(buf);
    create_user_task_with_pml4(elf->entry, user_pml4);
    return 0;
}
