#ifndef AHCI_H
#define AHCI_H
#include "types.h"

#define AHCI_BASE 0x400000

void ahci_init(void);
int ahci_read_sectors(uint64_t lba, uint32_t count, void* buf);
int ahci_write_sectors(uint64_t lba, uint32_t count, const void* buf);

#endif
