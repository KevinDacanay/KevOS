/**
 * @file irq.h
 * @brief Header for Interrupt Request (IRQ) management.
 *
 * Declares functions for installing custom IRQ handlers.
 */

#ifndef KERNEL_ARCH_I386_CPU_IRQ_H
#define KERNEL_ARCH_I386_CPU_IRQ_H

#include "idt.h"

extern "C" {
/**
 * @brief Installs a custom handler for a specific IRQ line.
 * @param irq The IRQ number (0-15) to install the handler for.
 * @param handler A function pointer to the custom handler.
 */
void irq_install_handler(int irq, void (*handler)(struct registers *r));
}

#endif