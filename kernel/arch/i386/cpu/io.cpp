#include "io.h"

extern "C" {

// Write a byte to a port
void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Read a byte from a port
uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Wait for I/O to complete (by writing to an unused port)
void io_wait(void) {
    // Port 0x80 is used for POST checkpoints and is safe to write to.
    // It's often used for I/O delays.
    asm volatile("outb %%al, $0x80" : : "a"(0));
}

} // extern "C"