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
#include <kernel/arch/i386/mm/include/heap.h>
#include <kernel/arch/i386/cpu/include/io.h> // For outb (reboot command)
#include <kernel/arch/i386/cpu/include/irq.h>
#include <kernel/arch/i386/cpu/include/gdt.h>
#include <kernel/arch/i386/mm/include/pmm.h>
#include <kernel/include/kernel/task.h> // Required for task_print_list

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
 * @brief Basic test suite for the Kernel Heap (kmalloc/kfree).
 */
static void test_heap() {
    printf("--- Heap Test Started ---\n");

    // Test 1: Simple Allocation
    void* a = kmalloc(1024); // 1KB
    void* b = kmalloc(1024); // 1KB
    printf("Allocated A (1KB) at 0x%x\n", (uint32_t)a);
    printf("Allocated B (1KB) at 0x%x\n", (uint32_t)b);

    // Test 2: Memory Read/Write (Verifies VMM Mapping)
    char* data = (char*)a;
    for(int i = 0; i < 10; i++) data[i] = 'K' + i;
    data[10] = '\0';
    printf("Data verification in A: %s\n", data);

    // Test 3: Freeing and Coalescing
    printf("Freeing A and B...\n");
    kfree(a);
    kfree(b);

    // Test 4: Dynamic Expansion
    // Our initial heap is 2MB. Let's allocate 4MB to force kheap_expand.
    printf("Requesting 4MB to trigger heap expansion...\n");
    void* c = kmalloc(4 * 1024 * 1024); 
    if (c) {
        printf("Allocated C (4MB) at 0x%x (Success!)\n", (uint32_t)c);
        kfree(c);
    } else {
        printf("Failed to allocate 4MB block!\n");
    }

    printf("--- Heap Test Completed Successfully ---\n");
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
        printf("  testheap    - Run a suite of tests on the kernel heap\n");
        printf("  uptime      - Displays the system uptime\n");
        printf("  gdtinfo     - Displays GDT and TSS CPU state\n");
        printf("  meminfo     - Displays physical memory usage\n");
        printf("  ps          - List all running tasks\n");
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
    else if (strcmp(cmd, "testheap") == 0) {
        test_heap();
    }
    else if (strcmp(cmd, "uptime") == 0) {
        uint32_t ticks = get_timer_ticks();
        uint32_t seconds = ticks / 100;
        printf("System uptime: %u seconds\n", seconds);
    }
    else if (strcmp(cmd, "gdtinfo") == 0) {
        uint16_t tr, cs, ds, ss;
        struct {
            uint16_t limit;
            uint32_t base;
        } __attribute__((packed)) gdtp;

        // Query the CPU for internal register states
        asm volatile("str %0" : "=r"(tr));   // Store Task Register
        asm volatile("sgdt %0" : "=m"(gdtp)); // Store GDT Pointer
        asm volatile("mov %%cs, %0" : "=r"(cs));
        asm volatile("mov %%ds, %0" : "=r"(ds));
        asm volatile("mov %%ss, %0" : "=r"(ss));

        printf("--- CPU GDT/TSS State ---\n");
        printf("GDT Pointer: Base=0x%x, Limit=0x%x\n", gdtp.base, gdtp.limit);
        printf("CS Selector: 0x%x (Expected: 0x08)\n", (uint32_t)cs);
        printf("DS Selector: 0x%x (Expected: 0x10)\n", (uint32_t)ds);
        printf("SS Selector: 0x%x (Expected: 0x10)\n", (uint32_t)ss);
        printf("Task Reg:    0x%x (Expected: 0x28)\n", (uint32_t)tr);
        
        if (ss != GDT_KERNEL_DATA_SEGMENT) {
            terminal_setcolor_colors(VGA_COLOR_RED, VGA_COLOR_BLACK);
            printf("WARNING: SS selector mismatch! Check gdt_flush assembly.\n");
            terminal_setcolor_colors(VGA_COLOR_MAGENTA, VGA_COLOR_BLACK);
        }

        if (tr == GDT_TSS_SEGMENT) {
            printf("Status: TSS is active and correctly loaded.\n");
        }
    }
    else if (strcmp(cmd, "meminfo") == 0) {
        size_t free_mem = pmm_get_free_memory();
        size_t total_mem = pmm_get_total_memory(); // Assuming pmm_get_total_memory() exists or can be added
        size_t used_mem = total_mem - free_mem;
        printf("--- Physical Memory Info ---\n");
        printf("Free Memory: %u KB\n", free_mem / 1024);
        printf("Used Memory: %u KB\n", used_mem / 1024);
    }
    else if (strcmp(cmd, "ps") == 0) {
        task_print_list();
    }
    else if (strcmp(cmd, "whatisthis") == 0) {
        printf("KevOS is a custom-built, 32-bit x86 operating system.\n");
        printf("It currently features a protected-mode kernel, VGA drivers,\n");
        printf("and an interrupt-driven keyboard subsystem.\n");
        printf("Next phase: Multitasking and Processes.\n");
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