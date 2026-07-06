#include "security.h"
#include "vga.h"
#include "string.h"

static user_context_t users[2];
static int current_user_idx = -1;

void security_init(void) {
    // 루트 계정
    users[0].uid = 0;
    strcpy(users[0].username, "root");
    users[0].permissions = 0xFFFFFFFF; // 모든 권한

    // 일반 사용자 계정
    users[1].uid = 1;
    strcpy(users[1].username, "user");
    users[1].permissions = PERM_FILE_READ | PERM_FILE_WRITE | PERM_NETWORK;

    current_user_idx = -1; // 로그인 안됨
    vga_print("[ OK ] Security Framework initialized.\n");
}

void security_login(int uid) {
    if (uid == 0 || uid == 1) {
        current_user_idx = uid;
    }
}

user_context_t* security_get_current_user(void) {
    if (current_user_idx != -1) {
        return &users[current_user_idx];
    }
    return NULL;
}

int security_check_permission(uint32_t perm) {
    if (current_user_idx == -1) return 0;
    if ((users[current_user_idx].permissions & perm) == perm) {
        return 1;
    }
    vga_print("[Security] Permission Denied!\n");
    return 0;
}
