/**
 * @file strcmp.cpp
 * @brief Implementation of string comparison.
 */

#include <string.h>

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    // Return the difference between the first non-matching characters
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}