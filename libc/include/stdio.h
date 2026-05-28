// Declares standard I/O functions
#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>  // Bring in internal definitions

#define EOF (-1)        // Standard EOF macro, returned on error by I/O functions

#ifdef __cplusplus
extern "C" {
#endif

// Print formatted output to screen
int printf(const char* __restrict, ...);    // __restrict is a compiler hint for optimization, it says the pointer doesn't alias others.

// Print a single character
int putchar(int);

// Print a string followed by newline
int puts(const char*);

#ifdef __cplusplus
}
#endif

#endif
