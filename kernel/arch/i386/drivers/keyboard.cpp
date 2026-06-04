/**
 * @file keyboard.cpp
 * @brief PS/2 Keyboard Driver implementation.
 *
 * This driver handles hardware interrupts from the keyboard (IRQ 1).
 * It translates raw scancodes from the PS/2 controller into ASCII characters,
 * maintaining internal state for modifier keys like Shift and Caps Lock.
 * Currently, it uses a simplified US-English layout mapping.
 */

#include "keyboard.h"
#include <kernel/arch/i386/cpu/io.h> // For inb
#include <kernel/arch/i386/cpu/pic.h> // For pic_eoi
#include <kernel/arch/i386/cpu/idt.h> // For idt_set_gate (to register handler) and struct registers
#include <kernel/arch/i386/cpu/irq.h> // For irq_install_handler
#include <stdio.h> // For printf
#include <kernel/tty.h> // For terminal_putchar

// Keyboard controller ports
#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64

// Buffer Configuration
#define KBD_BUFFER_SIZE 256
static char kbd_buffer[KBD_BUFFER_SIZE];
static uint32_t kbd_buffer_head = 0; ///< Write pointer for the ISR
static uint32_t kbd_buffer_tail = 0; ///< Read pointer for the kernel

// Keyboard State
static bool caps_lock = false;      ///< Tracks the toggle state of Caps Lock.
static bool lshift_pressed = false; ///< Tracks if the Left Shift key is currently held down.
static bool rshift_pressed = false; ///< Tracks if the Right Shift key is currently held down.

/**
 * @brief Scancode Set 1 (US-English) mapping table.
 * 
 * Maps raw 8-bit scancodes to their corresponding ASCII characters.
 * Index 0x01 is Escape, 0x0E is Backspace, 0x1C is Enter, etc.
 * 
 * Note: Special keys like Arrows, F-keys, and Numpad require extended 
 * scancode handling (0xE0 prefix) not yet fully implemented here.
 */
static const char kbd_us[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/**
 * @brief Scancode Set 1 (US-English) shifted mapping table.
 * 
 * Used when either Left or Right Shift is active.
 */
static const char kbd_us_shifted[128] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',   0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/**
 * @brief C-level Interrupt Service Routine for the keyboard.
 * 
 * This function is registered via the IRQ manager. It reads the raw scancode
 * from the I/O port, updates the internal modifier state, and translates
 * key presses into characters for the terminal.
 * 
 * @param regs The CPU register state at the time of the interrupt.
 */
extern "C" void keyboard_handler(struct registers* regs) {
    (void)regs;
    
    // Read the scancode from the keyboard data port
    uint8_t scancode = inb(KBD_DATA_PORT);

    // Handle modifier keys (Shift, Caps Lock). 
    // Scancodes with bit 7 set (0x80) represent key releases (break codes).
    switch (scancode) {
        case 0x2A: // Left Shift Pressed
            lshift_pressed = true;
            return;
        case 0xAA: // Left Shift Released
            lshift_pressed = false;
            return;
        case 0x36: // Right Shift Pressed
            rshift_pressed = true;
            return;
        case 0xB6: // Right Shift Released
            rshift_pressed = false;
            return;
        case 0x3A: // Caps Lock Pressed (Toggle)
            caps_lock = !caps_lock;
            return;
        default:
            break;
    }

    // Only handle key presses (bit 7 is 0 for press, 1 for release)
    if (!(scancode & 0x80)) {
        bool shift_active = lshift_pressed || rshift_pressed;
        char c = shift_active ? kbd_us_shifted[scancode] : kbd_us[scancode];

        if (c != 0) {
            // Apply Caps Lock logic for letters
            if (caps_lock && c >= 'a' && c <= 'z') c -= 32;
            else if (caps_lock && c >= 'A' && c <= 'Z') c += 32;

            if (c == '\b') {
                // Handle Backspace: Only remove from buffer if it's not empty
                if (kbd_buffer_head != kbd_buffer_tail) {
                    // Move head back (handling wrap-around)
                    kbd_buffer_head = (kbd_buffer_head - 1) % KBD_BUFFER_SIZE;
                    terminal_putchar(c); 
                }
            } else {
                // Handle standard character: Push to circular buffer
                uint32_t next = (kbd_buffer_head + 1) % KBD_BUFFER_SIZE;
                
                // Check for buffer overflow
                if (next != kbd_buffer_tail) {
                    kbd_buffer[kbd_buffer_head] = c;
                    kbd_buffer_head = next;
                    terminal_putchar(c); // Echo to screen
                }
            }
        }
    }

    // Note: pic_eoi is called by the common irq_handler in irq.cpp
}

extern "C" {

void keyboard_install(void) {
    irq_install_handler(1, keyboard_handler);
    printf("Keyboard driver installed. Type away...\n");
}

char keyboard_getchar(void) {
    // Block while buffer is empty
    while (kbd_buffer_head == kbd_buffer_tail) {
        // Using 'hlt' is battery-friendly; it stops the CPU until the next interrupt
        asm volatile("hlt");
    }

    // Fetch character from the tail
    char c = kbd_buffer[kbd_buffer_tail];
    // Advance the tail pointer
    kbd_buffer_tail = (kbd_buffer_tail + 1) % KBD_BUFFER_SIZE;
    
    return c;
}

} // extern "C"