#ifndef ELF_H
#define ELF_H
#include "types.h"

#define ELF_MAGIC 0x464C457F
#define PT_LOAD 1

typedef struct {
    uint32_t magic;
    uint8_t class;
    uint8_t data;
    uint8_t version;
    uint8_t osabi;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t hdr_size;
    uint16_t phent_size;
    uint16_t phnum;
} __attribute__((packed)) elf_header_t;

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
} __attribute__((packed)) elf_phdr_t;

int elf_load_and_exec(const char* path);

#endif
