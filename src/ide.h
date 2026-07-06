#ifndef IDE_H
#define IDE_H
#include "types.h"
int ide_read_sectors(uint64_t lba, uint32_t count, void* buf);
int ide_write_sectors(uint64_t lba, uint32_t count, const void* buf);
#endif
