// Copies 'size' bytes from 'src' to 'dst'.
// Undefined behavior if regions overlap. For overlapping memory, use memmove.

#include <string.h>

void* memcpy(void* __restrict dstptr, const void* __restrict srcptr, size_t size) {
    unsigned char* dst = (unsigned char*) dstptr;
    const unsigned char* src = (const unsigned char*) srcptr;
    for (size_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }
    return dstptr;
}