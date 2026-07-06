#ifndef CLOUDPKG_H
#define CLOUDPKG_H
#include "types.h"

int cloudpkg_install(const char* pkg_name);
int cloudpkg_rollback(const char* pkg_name);

#endif
