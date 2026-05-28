// Safely copies 'size' bytes from 'src' to 'dst', handling overlapping memory regions.

#include <string.h>

void* memmove(void* dstptr, const void* srcptr, size_t size) {
    unsigned char* dst = (unsigned char*) dstptr;
    const unsigned char* src = (const unsigned char*) srcptr;
    if (dst < src) {
        // Safe to copy forward
        for (size_t i = 0; i < size; i++) {
            dst[i] = src[i];
        } 
    } else {
        // Copy backwards to avoid overwriting source
        for (size_t i = size; i != 0; i--) {
            dst[i - 1] = src[i - 1];
        }
    }
    return dstptr;
}
