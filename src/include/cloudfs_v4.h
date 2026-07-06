#ifndef CLOUDFS_V4_H
#define CLOUDFS_V4_H
#include "types.h"

void cloudfs_v4_init(void);
int cloudfs_cow_write(int inode, const void* buf, uint32_t size);
int cloudfs_snapshot(const char* path);

#endif
