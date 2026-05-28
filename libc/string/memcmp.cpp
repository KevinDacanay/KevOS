// Compares two blocks of memory byte by byte.
// Returns:
//   -1 if first differing byte in 'a' is less than in 'b'
//    0 if blocks are equal
//    1 if first differing byte in 'a' is greater than in 'b'

#include <string.h>

int memcmp(const void* aptr, const void* bptr, size_t size) {
    const unsigned char* a = (const unsigned char*) aptr;
    const unsigned char* b = (const unsigned char*) bptr;
    for (size_t i = 0; i < size; i++) {
        if (a[i] < b[i]) {
            return -1;
        } else if (b[i] < a[i]) {
            return 1;
        }
    }
    return 0;
}