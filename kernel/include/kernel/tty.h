#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>

// Initializes the terminal screen (clears it).
void terminal_initialize(void);

// Writes a single character to the screen at the current cursor.
void terminal_putchar(char c);

// Writes a raw buffer to the screen.
void terminal_write(const char* data, size_t size);

// Writes a null-terminated string to the screen.
void terminal_writestring(const char* data);

#endif
