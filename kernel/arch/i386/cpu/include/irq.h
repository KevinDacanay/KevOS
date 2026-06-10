/**
 * @file irq.h
 * @brief Header for Interrupt Request (IRQ) management.
 *
 * Declares functions for installing custom IRQ handlers.
 */

#ifndef KERNEL_ARCH_I386_CPU_IRQ_H
#define KERNEL_ARCH_I386_CPU_IRQ_H

#include <stdint.h>
#include "idt.h"

extern "C" {
/**
 * @brief Installs a custom handler for a specific IRQ line.
 * @param irq The IRQ number (0-15) to install the handler for.
 * @param handler A function pointer to the custom handler.
 */
void irq_install_handler(int irq, void (*handler)(struct registers *r));

/**
 * @brief Returns the total number of system ticks since the PIT was initialized.
 * @return uint32_t The current tick count.
 */
uint32_t get_timer_ticks();
}

#endif