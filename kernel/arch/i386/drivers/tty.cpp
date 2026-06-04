/**
 * @file tty.cpp
 * @brief VGA Text Mode Terminal Driver.
 *
 * This file implements the kernel terminal (TTY). It manages the VGA text buffer
 * located at physical address 0xB8000, handling character output, escape sequences
 * (like newlines and backspaces), and vertical scrolling.
 */

#include <stdbool.h>
#include <string.h>
#include <kernel/arch/i386/cpu/io.h>
#include <kernel/include/kernel/tty.h>

/**
 * @brief Combines a foreground and background color into a single 8-bit attribute.
 */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

/**
 * @brief Combines an ASCII character and an 8-bit color attribute into a 16-bit VGA entry.
 */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

// Terminal dimensions for standard 80x25 text mode
static const size_t VGA_WIDTH = 80;   ///< Columns
static const size_t VGA_HEIGHT = 25;  ///< Rows

static size_t terminal_row;           ///< Current cursor Y position
static size_t terminal_column;        ///< Current cursor X position
static uint8_t terminal_color;        ///< Current text attribute (color)
static uint16_t* terminal_buffer;     ///< Pointer to the VGA memory (0xB8000)

/**
 * @brief Updates the hardware cursor position on the screen.
 * 
 * Tells the VGA controller where to render the blinking cursor based
 * on the current terminal_row and terminal_column.
 */
void update_cursor() {
	uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

/**
 * @brief Configures the hardware cursor shape and enables it.
 * 
 * @param cursor_start Top scanline (0-15).
 * @param cursor_end Bottom scanline (0-15).
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

/**
 * @brief Clears the screen and initializes the terminal state.
 * 
 * Sets the cursor to (0,0), defines the default color (Light Grey on Black),
 * and fills the entire VGA buffer with space characters.
 */
void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK); // Default color
	terminal_buffer = (uint16_t*) 0xB8000; // Physical address for VGA text mode buffer

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}

	// Enable a block cursor (0-15 scanlines)
	enable_cursor(0, 15);
	update_cursor();
}

/**
 * @brief Sets the current foreground and background colors for the terminal.
 * @param fg The foreground color.
 * @param bg The background color.
 */
void terminal_setcolor_colors(enum vga_color fg, enum vga_color bg) {
    terminal_color = vga_entry_color(fg, bg);
}

/**
 * @brief Scrolls the terminal content up by one line.
 * 
 * Moves all character entries from rows [1..24] to [0..23] and 
 * clears the bottom row with the current terminal color.
 */
void terminal_scroll() {
    // Shift lines up
	for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
		}
	}
    // Clear the last line
	for (size_t x = 0; x < VGA_WIDTH; x++) {
		terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
	}
}

/**
 * @brief Writes a single character to the terminal.
 * 
 * Handles special characters like '\n' (Newline) and '\b' (Backspace).
 * Automatically triggers scrolling if the cursor exceeds the bottom row.
 * 
 * @param c The character to display.
 */
void terminal_putchar(char c) {
    // Handle Newline
	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			terminal_scroll();
			terminal_row = VGA_HEIGHT - 1;
		}
		update_cursor();
		return;
	}

    // Handle Backspace
	if (c == '\b') {
		if (terminal_column > 0) {
			terminal_column--;
		} else if (terminal_row > 0) {
			terminal_row--;
			terminal_column = VGA_WIDTH - 1;
		}

		// Erase the character at the new cursor position
		const size_t index = terminal_row * VGA_WIDTH + terminal_column;
		terminal_buffer[index] = vga_entry(' ', terminal_color);
		update_cursor();
		return;
	}

    // Standard character output
	const size_t index = terminal_row * VGA_WIDTH + terminal_column;
	terminal_buffer[index] = vga_entry(c, terminal_color);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			terminal_scroll();
			terminal_row = VGA_HEIGHT - 1;
		}
	}
	update_cursor();
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}