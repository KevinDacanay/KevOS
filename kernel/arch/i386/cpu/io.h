#ifndef KERNEL_ARCH_I386_CPU_IO_H
#define KERNEL_ARCH_I386_CPU_IO_H

#include <stdint.h>

extern "C" {
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void io_wait(void);
}

#endif // KERNEL_ARCH_I386_CPU_IO_H