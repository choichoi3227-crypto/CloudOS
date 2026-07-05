#include "vfs.h"
#include "heap.h"
#include "string.h"

struct file_node {
    char name[32];
    void* data;
    uint32_t size;
    int used;
};

static struct file_node file_table[MAX_FILES];

void vfs_init(void) {
    for(int i=0; i<MAX_FILES; i++) {
        file_table[i].used = 0;
    }
}

void vfs_write_file(const char* name, void* buf, uint32_t size) {
    for(int i=0; i<MAX_FILES; i++) {
        if(!file_table[i].used) {
            strcpy(file_table[i].name, name);
            file_table[i].data = kmalloc(size);
            memcpy(file_table[i].data, buf, size);
            file_table[i].size = size;
            file_table[i].used = 1;
            return;
        }
    }
}

uint32_t vfs_read_file(const char* name, void* buf, uint32_t size) {
    for(int i=0; i<MAX_FILES; i++) {
        if(file_table[i].used && strcmp(file_table[i].name, name) == 0) {
            uint32_t read_size = file_table[i].size < size ? file_table[i].size : size;
            memcpy(buf, file_table[i].data, read_size);
            return read_size;
        }
    }
    return 0;
}
