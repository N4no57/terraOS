#include "headers/utils.h"

void* memcpy(void* dest, const void* src, size_t count) {
    char* d = dest;
    const char* s = src;
    for (int i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

void *memset(void* dest, int c, size_t count) {
    u8 *ptr = (u8*) dest;
    u8 byte_val = (u8) c;
    for (size_t i = 0; i < count; i++) {
        ptr[i] = byte_val;
    }
    return dest;
}