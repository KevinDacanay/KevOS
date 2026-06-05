/**
 * @file io.cpp
 * @brief Basic I/O port access functions for x86.
 *
 * This file provides functions to interact with hardware I/O ports
 * using `inb` (read byte) and `outb` (write byte) instructions.
 * It also includes a utility for I/O delays.
 */

#include <kernel/arch/i386/cpu/include/io.h>

extern "C" {

/**
 * @brief Writes a byte to the specified I/O port.
 * @param port The 16-bit I/O port address.
 * @param value The 8-bit value to write.
 */
void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * @brief Reads a byte from the specified I/O port.
 * @param port The 16-bit I/O port address.
 * @return The 8-bit value read from the port.
 */
uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * @brief Introduces a short delay by performing a dummy I/O operation.
 *
 * This function writes to I/O port 0x80, which is typically unused and
 * serves as a safe way to ensure I/O operations have completed or to
 * introduce a minimal delay.
 */
void io_wait(void) {
    asm volatile("outb %%al, $0x80" : : "a"(0));
}

} // extern "C"