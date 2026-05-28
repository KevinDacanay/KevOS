#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

// Raise a signal to the current process
int raise(int sig) {
#if defined(__is_libk)
    // For now, we only handle SIGABRT in the kernel
    if (sig == SIGABRT) {
        abort();  // invoke abort from <stdlib.h>
        return 0;
    }
    printf("Kernel: Unhandled signal %d\n", sig);
    return 1;
#else
    // User-mode: TODO - Implement system call to deliver signal to process
    printf("raise(%d)\n", sig);
    // This is a stub: should terminate or call a handler
    abort();  // fallback behavior
    return 0;
#endif
}

/*
TO-DO: Implement signal() for when OS is multi-tasking and signal aware

// src/stdlib/signal.cpp

#include <signal.h>
#include <stddef.h>

static __sighandler_t signal_handlers[32]; // Up to 32 signal types

__sighandler_t signal(int sig, __sighandler_t handler) {
    if (sig <= 0 || sig >= 32) {
        return (__sighandler_t)0; // Invalid signal
    }
    __sighandler_t old = signal_handlers[sig];
    signal_handlers[sig] = handler;
    return old;
}

*/