// src/fs/vfs.c (새 파일 생성)
#include "fs/vfs.h"
#include "../include/string.h"

static vfs_node_t root_node;

void vfs_init(void) {
    memset(&root_node, 0, sizeof(vfs_node_t));
    root_node.type = VFS_DIRECTORY;
    strcpy(root_node.name, "/");
}

static vfs_node_t* find_child_dir(vfs_node_t* dir, const char* name) {
    for (int i = 0; i < dir->child_count; i++) {
        if (strcmp(dir->children[i]->name, name) == 0) return dir->children[i];
    }
    return NULL;
}

vfs_node_t* vfs_find_node(const char* path) {
    if (!path || path[0] != '/') return NULL;
    if (strcmp(path, "/") == 0) return &root_node;

    vfs_node_t* current = &root_node;
    char temp_name[VFS_MAX_FILENAME];
    const char *ptr = path + 1;

    while (*ptr) {
        int i = 0;
        while (*ptr && *ptr != '/' && i < VFS_MAX_FILENAME - 1) {
            temp_name[i++] = *ptr++;
        }
        temp_name[i] = '\0';
        
        if (*ptr == '/') ptr++; // 슬래시 건너뛰기
        
        if (temp_name[0] == '\0') continue;
        
        if (current->type != VFS_DIRECTORY) return NULL;
        current = find_child_dir(current, temp_name);
        if (!current) return NULL;
    }
    return current;
}

vfs_node_t* vfs_create_dir(const char* path) {
    vfs_node_t* parent = vfs_find_node(path);
    if (parent && parent->type == VFS_FILE) return NULL; // 파일 안에 디렉토리 못 만듦
    
    // 부모 경로 추출
    char parent_path[256];
    strcpy(parent_path, path);
    char* last_slash = strrchr(parent_path, '/');
    if (last_slash) *last_slash = '\0'; else strcpy(parent_path, "/");
    
    parent = vfs_find_node(parent_path);
    const char* dir_name = strrchr(path, '/') + 1;
    if (!parent || !dir_name || parent->child_count >= VFS_MAX_CHILDREN) return NULL;
    
    vfs_node_t* new_dir = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!new_dir) return NULL;
    memset(new_dir, 0, sizeof(vfs_node_t));
    strcpy(new_dir->name, dir_name);
    new_dir->type = VFS_DIRECTORY;
    new_dir->parent = parent;
    
    parent->children[parent->child_count++] = new_dir;
    return new_dir;
}

vfs_node_t* vfs_create_file(const char* path, const char* content) {
    char parent_path[256];
    strcpy(parent_path, path);
    char* last_slash = strrchr(parent_path, '/');
    const char* filename = last_slash ? last_slash + 1 : path;
    if (last_slash) *last_slash = '\0'; else strcpy(parent_path, "/");
    
    vfs_node_t* parent = vfs_find_node(parent_path);
    if (!parent || parent->type != VFS_DIRECTORY || !filename[0]) return NULL;
    if (find_child_dir(parent, filename)) return NULL; // 이미 있음
    
    vfs_node_t* new_file = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!new_file) return NULL;
    memset(new_file, 0, sizeof(vfs_node_t));
    strcpy(new_file->name, filename);
    new_file->type = VFS_FILE;
    new_file->parent = parent;
    
    if (content) {
        new_file->size = strlen(content);
        new_file->data = (uint8_t*)kmalloc(new_file->size + 1);
        strcpy((char*)new_file->data, content);
    }
    
    parent->children[parent->child_count++] = new_file;
    return new_file;
}

const char* vfs_read_file(const char* path, uint32_t* out_size) {
    vfs_node_t* node = vfs_find_node(path);
    if (!node || node->type != VFS_FILE || !node->data) return NULL;
    if (out_size) *out_size = node->size;
    return (const char*)node->data;
}
