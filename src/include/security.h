#ifndef SECURITY_H
#define SECURITY_H
#include "types.h"

// 권한 비트마스크
#define PERM_FILE_READ  0x01
#define PERM_FILE_WRITE 0x02
#define PERM_NETWORK    0x04
#define PERM_DEVICE     0x08

typedef struct {
    int uid;            // 0: root, 1: user
    char username[32];
    uint32_t permissions;
} user_context_t;

void security_init(void);
void security_login(int uid);
user_context_t* security_get_current_user(void);
int security_check_permission(uint32_t perm);

#endif
