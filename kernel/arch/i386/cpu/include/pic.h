/**
 * @file pic.h
 * @brief Header for Programmable Interrupt Controller (PIC) management.
 *
 * Declares functions for remapping and sending End-Of-Interrupt (EOI) signals to the PICs.
 */

#ifndef KERNEL_ARCH_I386_CPU_PIC_H
#define KERNEL_ARCH_I386_CPU_PIC_H

#include <stdint.h>

extern "C" {
/** @brief Remaps the PICs to new interrupt vector offsets. */
void pic_remap(int offset1, int offset2);
/** @brief Sends an End-Of-Interrupt (EOI) signal to the PICs. */
void pic_eoi(uint8_t irq);
}

#endif // KERNEL_ARCH_I386_CPU_PIC_H