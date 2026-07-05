#include "elf.h"
#include "vfs.h"
#include "vmm.h"
#include "pmm.h"
#include "heap.h"
#include "task.h"
#include "vga.h"
#include "string.h"

int elf_load_and_exec(const char* path) {
    uint8_t* buf = (uint8_t*)kmalloc(4096);
    if (!buf) return -1;
    
    uint32_t read_bytes = vfs_read_file(path, buf, 4096);
    if (read_bytes <= 0) {
        vga_print("[ELF] File not found: ");
        vga_print(path);
        vga_print("\n");
        kfree(buf);
        return -1;
    }

    elf_header_t* elf = (elf_header_t*)buf;
    if (elf->magic != ELF_MAGIC) {
        vga_print("[ELF] Invalid ELF magic!\n");
        kfree(buf);
        return -1;
    }

    vga_print("[ELF] Loading ELF: ");
    vga_print(path);
    vga_print("\n");

    // 새로운 사용자 주소 공간 생성
    uint64_t user_pml4 = vmm_create_user_pml4();
    
    // Program Header 순회
    elf_phdr_t* phdr = (elf_phdr_t*)(buf + elf->phoff);
    for (int i = 0; i < elf->phnum; i++) {
        if (phdr[i].type == PT_LOAD) {
            uint32_t pages = (phdr[i].memsz + 4095) / 4096;
            for (uint32_t p = 0; p < pages; p++) {
                physaddr_t phys = pmm_alloc_block();
                // 사용자 공간 가상 주소에 매핑 (User | Writable | Present)
                vmm_map_page_to_pml4(user_pml4, phys, phdr[i].vaddr + p * 4096, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
                
                // 파일 내용을 물리 메모리로 복사
                // (간략화: 실제로는 커널 가상 주소를 통해 복사해야 하나, 여기서는 1:1 매핑 가정)
                memcpy((void*)phys, buf + phdr[i].offset + p * 4096, 4096);
            }
        }
    }

    kfree(buf);

    // 새 태스크 생성 및 페이지 테이블 전환 후 사용자 모드 진입
    create_user_task_with_pml4(elf->entry, user_pml4);
    return 0;
}
