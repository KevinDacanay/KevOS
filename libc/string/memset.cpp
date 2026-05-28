// Sets 'size' bytes in the memory block pointed to by 'bufptr' to the specified 'value'.

#include <string.h>

void* memset(void* bufptr, int value, size_t size) {
    unsigned char* buf = (unsigned char*) bufptr;
    for (size_t i = 0; i < size; i++) {
        buf[i] = (unsigned char) value;
    }
    return bufptr;
}