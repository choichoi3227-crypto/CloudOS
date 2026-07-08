// src/fs/vfs.h (새 파일 생성)
#ifndef VFS_H
#define VFS_H

#include "../include/types.h"

#define VFS_MAX_FILENAME 32
#define VFS_MAX_CHILDREN 16

typedef enum {
    VFS_FILE,
    VFS_DIRECTORY
} vfs_node_type_t;

typedef struct vfs_node {
    char name[VFS_MAX_FILENAME];
    vfs_node_type_t type;
    uint32_t size;
    uint8_t *data; // 파일 내용 (메모리 포인터)
    
    struct vfs_node *parent;
    struct vfs_node *children[VFS_MAX_CHILDREN];
    int child_count;
} vfs_node_t;

// 가상 파일 시스템 초기화 (루트 디렉토리 생성)
void vfs_init(void);

// 파일 조작
vfs_node_t* vfs_create_file(const char* path, const char* content);
vfs_node_t* vfs_create_dir(const char* path);
vfs_node_t* vfs_find_node(const char* path);
const char* vfs_read_file(const char* path, uint32_t* out_size);

#endif
