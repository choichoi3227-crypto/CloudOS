#include "vfs.h"
#include "cloudfs.h"

void vfs_init(void) {
    cloudfs_init();
}

void vfs_write_file(const char* name, void* buf, uint32_t size) {
    cloudfs_write_file(name, buf, size);
}

uint32_t vfs_read_file(const char* name, void* buf, uint32_t size) {
    return cloudfs_read_file(name, buf, size);
}

void vfs_list_files(void) {
    cloudfs_list_files();
}
