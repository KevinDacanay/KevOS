/**
 * @file shell.cpp
 * @brief Implementation of the KevOS kernel shell.
 * 
 * This module handles command parsing and execution. It relies on the 
 * keyboard driver for input and the TTY driver for output.
 */

#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <kernel/tty.h>
#include <kernel/arch/i386/drivers/keyboard.h>

/**
 * @brief Reads a line of input from the keyboard.
 * 
 * @param buffer The buffer to store the input string.
 * @param max_len The maximum number of characters to read.
 */
void shell_readline(char* buffer, size_t max_len) {
    size_t i = 0;
    while (i < max_len - 1) {
        char c = keyboard_getchar();

        if (c == '\n') {
            terminal_putchar('\n');
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                terminal_putchar('\b'); // Visually erase character
            }
        } else {
            buffer[i++] = c;
            terminal_putchar(c); // Echo character to screen
        }
    }
    buffer[i] = '\0'; // Null-terminate the string
}

/**
 * @brief Parses and executes a single command string.
 * 
 * @param cmd The raw command string entered by the user.
 */
void shell_execute(char* cmd) {
    if (strlen(cmd) == 0) {
        return;
    }

    if (strcmp(cmd, "help") == 0) {
        printf("Available commands:\n");
        printf("  help        - Show this help message\n");
        printf("  clear       - Clear the screen\n");
        printf("  whatisthis  - Explain this project\n");
        printf("  exit        - Close the shell and halt project\n");
        printf("  quit        - Close the shell and halt project\n");
    } 
    else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
        printf("System halting... Safe to power off.\n");
        for (;;) {
            asm volatile("hlt");
        }
    }
    else if (strcmp(cmd, "clear") == 0) {
        terminal_initialize();
    } 
    else if (strcmp(cmd, "whatisthis") == 0) {
        printf("KevOS is a custom-built, 32-bit x86 operating system.\n");
        printf("It currently features a protected-mode kernel, VGA drivers,\n");
        printf("and an interrupt-driven keyboard subsystem.\n");
        printf("Next phase: Physical Memory Management (PMM).\n");
    } 
    else {
        printf("Unknown command: %s\n", cmd);
    }
}

/**
 * @brief The main entry point for the shell.
 */
extern "C" void shell_main() {
    char buffer[128];

    printf("\n--- KevOS Shell v0.1 ---\n");
    printf("Type 'help' for a list of commands.\n\n");

    while (true) {
        printf("kevos> ");
        
        // Read a line of input
        shell_readline(buffer, 128);
        
        // Execute the command
        shell_execute(buffer);
        
        printf("\n");
    }
}