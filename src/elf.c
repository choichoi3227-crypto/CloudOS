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

    uint64_t user_pml4 = vmm_create_user_pml4();
    
    elf_phdr_t* phdr = (elf_phdr_t*)(buf + elf->phoff);
    for (int i = 0; i < elf->phnum; i++) {
        if (phdr[i].type == PT_LOAD) {
            uint32_t pages = (phdr[i].memsz + 4095) / 4096;
            for (uint32_t p = 0; p < pages; p++) {
                physaddr_t phys = pmm_alloc_block();
                vmm_map_page_to_pml4(user_pml4, phys, phdr[i].vaddr + p * 4096, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
                memcpy((void*)phys, buf + phdr[i].offset + p * 4096, 4096);
            }
        }
    }

    kfree(buf);
    create_user_task_with_pml4(elf->entry, user_pml4);
    return 0;
}
