// --- putchar.cpp ---
#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#endif

// putchar: writes a single character to the output (typically the screen)
int putchar(int ic) {
#if defined(__is_libk)
    // Convert integer to char and write it using kernel terminal output
    char c = (char) ic;
    terminal_write(&c, sizeof(c));
#else
    // TODO: implement stdio and write system call
    // We need to implement a syscall (e.g., write to file descriptor 1 - stdout)
    // Example: syscall_write(1, &ic, 1);
    // For now, we will just return the character without output
#endif
    return ic;
}
