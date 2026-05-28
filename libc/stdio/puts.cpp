// --- puts.cpp ---
#include <stdio.h>

// puts: prints a null-terminated string followed by a newline
int puts(const char* string) {
    return printf("%s\n", string); // Appends newline automatically
}
