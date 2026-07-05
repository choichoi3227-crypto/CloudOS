#ifndef VFS_H
#define VFS_H
#include "types.h"

void vfs_init(void);
void vfs_write_file(const char* name, void* buf, uint32_t size);
uint32_t vfs_read_file(const char* name, void* buf, uint32_t size);
void vfs_list_files(void);

#endif
