#ifndef STRING_H
#define STRING_H
#include "types.h"
int strcmp(const char* a, const char* b);
uint64_t strlen(const char* str);
void* memcpy(void* dest, const void* src, uint64_t n);
void* memset(void* ptr, int value, uint64_t num);
int strncmp(const char* a, const char* b, uint64_t n);
#endif
