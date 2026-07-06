#ifndef NVME_H
#define NVME_H
#include "types.h"

void nvme_init(void);
int nvme_read_sectors(uint64_t lba, uint32_t count, void* buf);
int nvme_write_sectors(uint64_t lba, uint32_t count, const void* buf);

#endif
