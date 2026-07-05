#include "string.h"

int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

uint64_t strlen(const char* str) {
    uint64_t len = 0;
    while (str[len]) len++;
    return len;
}
