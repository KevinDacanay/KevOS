// Declares utility functions 
#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

// Causes abnormal program termination. Does not return. (kernel panic)
__attribute__((__noreturn__))   
void abort(void);   // fatal error handler

#ifdef __cplusplus
}
#endif

#endif
