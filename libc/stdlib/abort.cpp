#include <stdio.h>
#include <stdlib.h>
#include <signal.h> // Required for raising signals in user mode

// Mark the function as non-returning so the compiler knows execution stops here
__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
    // KERNEL MODE IMPLEMENTATION

    // Log a panic message to the terminal
    printf("Kernel: panic: abort()\n");

    // Halt the CPU indefinitely to prevent further execution
    asm volatile("cli");
    asm volatile("hlt");
#else
    // USER MODE IMPLEMENTATION

    // Log the abort message (you can replace this with writing to stderr later)
    printf("abort()\n");

    // Raise the SIGABRT signal to terminate the process abnormally
    raise(SIGABRT);
#endif

    // If for some reason execution continues, enter an infinite loop
    while (1) {}

    // Hint to the compiler that this code path is unreachable
    __builtin_unreachable();
}