// Declares standard string and memory functions
#ifndef _STRING_H
#define _STRING_H

#include <sys/cdefs.h>
#include <stddef.h> // for size_t

#ifdef __cplusplus
extern "C" {
#endif

// Compare memory regions
int memcmp(const void*, const void*, size_t);

// Copy memory block (not safe for overlapping memory)
void* memcpy(void* __restrict, const void* __restrict, size_t);

// Copy memory safely even if regions overlap
void* memmove(void*, const void*, size_t);

// Fill memory block with value (usually 0)
void* memset(void*, int, size_t);

// Get string length (not including null terminator)
size_t strlen(const char*);

#ifdef __cplusplus
}
#endif

#endif
