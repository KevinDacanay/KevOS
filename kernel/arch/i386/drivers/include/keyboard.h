/**
 * @file keyboard.h
 * @brief Public interface for the PS/2 keyboard driver.
 */

#ifndef KERNEL_ARCH_I386_DRIVERS_KEYBOARD_H
#define KERNEL_ARCH_I386_DRIVERS_KEYBOARD_H

extern "C" {
/**
 * @brief Initializes the keyboard driver.
 * 
 * Registers the keyboard ISR with IRQ 1 and prints a status message.
 */
void keyboard_install(void);

/**
 * @brief Reads a single character from the keyboard buffer.
 * 
 * This is a blocking call. If the buffer is empty, it will halt the CPU
 * until an interrupt populates the buffer.
 */
char keyboard_getchar(void);
}

#endif // KERNEL_ARCH_I386_DRIVERS_KEYBOARD_H