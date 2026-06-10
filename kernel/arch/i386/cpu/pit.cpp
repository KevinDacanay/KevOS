/**
 * @file pit.cpp
 * @brief Programmable Interval Timer (PIT) driver.
 *
 * This module initializes and configures the Intel 8253/8254 PIT chip
 * to generate periodic interrupts (IRQ0) at a specified frequency.
 */

#include <kernel/arch/i386/cpu/include/pit.h>
#include <kernel/arch/i386/cpu/include/io.h> // For outb
#include <kernel/arch/i386/cpu/include/irq.h> // For irq_install_handler
#include <stdio.h>

// The base frequency of the PIT is 1193180 Hz
#define PIT_BASE_FREQUENCY 1193180

/**
 * @brief Initializes the Programmable Interval Timer (PIT).
 * 
 * Configures PIT Channel 0 to operate in Square Wave Generator mode (Mode 3)
 * and sets its frequency.
 * 
 * @param frequency The desired frequency in Hz (e.g., 100 for 100Hz).
 */
void pit_init(uint32_t frequency) {
    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;
    outb(PIT_COMMAND_PORT, 0x36); // Channel 0, Access mode lobyte/hibyte, Mode 3 (square wave), Binary
    outb(PIT_CHANNEL0_DATA, divisor & 0xFF); // Low byte
    outb(PIT_CHANNEL0_DATA, (divisor >> 8) & 0xFF); // High byte
    printf("PIT initialized to %d Hz.\n", frequency);
}