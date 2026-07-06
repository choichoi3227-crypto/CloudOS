#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H
#include "types.h"

void subsystem_init(void);
int exec_windows_app(const char* path);
int exec_linux_app(const char* path);

#endif
