/**
 * @file shell.cpp
 * @brief Implementation of the KevOS kernel shell.
 * 
 * This module handles command parsing and execution. It relies on the 
 * keyboard driver for input and the TTY driver for output.
 */

#include <kernel/arch/i386/drivers/include/shell.h>
#include <stdio.h>
#include <string.h> // For strlen, strcmp
#include <stdlib.h> // For abort()
#include <kernel/tty.h>
#include <kernel/arch/i386/drivers/include/keyboard.h>
#include <kernel/arch/i386/cpu/include/io.h> // For outb (reboot command)

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
 * @brief Parses a color name string into a vga_color enum value.
 * @param color_name The string representation of the color (e.g., "red", "blue").
 * @return The corresponding vga_color enum value, or VGA_COLOR_BLACK if invalid.
 */
static enum vga_color parse_color_name(const char* color_name) {
    if (strcmp(color_name, "black") == 0) return VGA_COLOR_BLACK;
    if (strcmp(color_name, "blue") == 0) return VGA_COLOR_BLUE;
    if (strcmp(color_name, "green") == 0) return VGA_COLOR_GREEN;
    if (strcmp(color_name, "cyan") == 0) return VGA_COLOR_CYAN;
    if (strcmp(color_name, "red") == 0) return VGA_COLOR_RED;
    if (strcmp(color_name, "magenta") == 0) return VGA_COLOR_MAGENTA;
    if (strcmp(color_name, "brown") == 0) return VGA_COLOR_BROWN;
    if (strcmp(color_name, "lightgrey") == 0) return VGA_COLOR_LIGHT_GREY;
    if (strcmp(color_name, "darkgrey") == 0) return VGA_COLOR_DARK_GREY;
    if (strcmp(color_name, "lightblue") == 0) return VGA_COLOR_LIGHT_BLUE;
    if (strcmp(color_name, "lightgreen") == 0) return VGA_COLOR_LIGHT_GREEN;
    if (strcmp(color_name, "lightcyan") == 0) return VGA_COLOR_LIGHT_CYAN;
    if (strcmp(color_name, "lightred") == 0) return VGA_COLOR_LIGHT_RED;
    if (strcmp(color_name, "lightmagenta") == 0) return VGA_COLOR_LIGHT_MAGENTA;
    if (strcmp(color_name, "lightbrown") == 0) return VGA_COLOR_LIGHT_BROWN;
    if (strcmp(color_name, "white") == 0) return VGA_COLOR_WHITE;
    return VGA_COLOR_BLACK; // Default or error color
}

/**
 * @brief Reboots the system using the keyboard controller.
 * 
 * This is a common method for software-initiated reboots in x86 systems.
 */
static void reboot_system() {
    uint8_t good = 0x02;
    // Wait for keyboard controller to be ready (bit 1 clear)
    // This loop ensures the controller is not busy before sending commands.
    while (good & 0x02) {
        good = inb(0x64); 
    }
    // Send reset command to keyboard controller (0xFE is the reset byte)
    outb(0x64, 0xFE); 
    // If the above fails, the system will likely hang here.
    // Enter an infinite loop to prevent further execution in case of failure.
    for (;;) {
        asm volatile("hlt");
    }
}

/**
 * @brief Parses and executes a single command string.
 * 
 * @param cmd The raw command string entered by the user.
 */
void shell_execute(char* cmd) {
    size_t len = strlen(cmd);
    if (len == 0) {
        return;
    }

    char* args = nullptr;
    // Find the first space to separate the command name from arguments
    for (size_t i = 0; i < len; i++) {
        if (cmd[i] == ' ') {
            cmd[i] = '\0';      // Null-terminate the command name
            args = &cmd[i + 1]; // Pointer to the start of arguments
            break;
        }
    }

    if (strcmp(cmd, "help") == 0) {
        printf("Available commands:\n");
        printf("  help        - Show this help message\n");
        printf("  clear       - Clear the screen\n");
        printf("  whatisthis  - Explain this project\n");
        printf("  exit        - Close the shell and halt project\n");
        printf("  quit        - Close the shell and halt project\n");
        printf("  echo <text> - Prints the given text\n");
        printf("  version     - Displays KevOS version\n");
        printf("  setcolor <fg> <bg> - Sets terminal foreground and background colors\n");
        printf("  reboot      - Reboots the system\n");
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
    else if (strcmp(cmd, "echo") == 0) {
        if (args) {
            printf("%s\n", args);
        } else {
            printf("\n");
        }
    }
    else if (strcmp(cmd, "version") == 0) {
        printf("KevOS Version 0.1 (i386 Protected Mode)\n");
    }
    else if (strcmp(cmd, "setcolor") == 0) {
        if (!args) {
            printf("Usage: setcolor <fg> <bg>\n");
            return;
        }

        char* fg_str = args;
        char* bg_str = nullptr;

        // Split args into two color strings
        for (size_t i = 0; fg_str[i] != '\0'; i++) {
            if (fg_str[i] == ' ') {
                fg_str[i] = '\0';
                bg_str = &fg_str[i + 1];
                break;
            }
        }

        if (!bg_str) {
            printf("Usage: setcolor <fg> <bg>\n");
            return;
        }

        enum vga_color fg = parse_color_name(fg_str);
        enum vga_color bg = parse_color_name(bg_str);

        // Basic validation: if parse_color_name returns VGA_COLOR_BLACK for an invalid name,
        // this check might be too simple. For now, assume valid names are used.
        terminal_setcolor_colors(fg, bg);
    }
    else if (strcmp(cmd, "reboot") == 0) {
        printf("Rebooting system...\n");
        reboot_system();
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