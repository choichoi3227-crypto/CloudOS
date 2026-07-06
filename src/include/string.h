#ifndef STRING_H
#define STRING_H
#include "types.h"

int strcmp(const char* a, const char* b);
char* strcpy(char* dest, const char* src);
uint64_t strlen(const char* str);
void* memcpy(void* dest, const void* src, uint64_t n);
void* memset(void* ptr, int value, uint64_t num);

#endif
