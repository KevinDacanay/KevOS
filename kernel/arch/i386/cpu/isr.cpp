/**
 * @file isr.cpp
 * @brief Interrupt Service Routine (ISR) handler for CPU exceptions.
 *
 * This file contains the common C-level handler for CPU exceptions (ISRs 0-31).
 * It is called by the assembly `isr_common_stub` after saving the CPU state.
 */

#include "idt.h"
#include <stdio.h>

/**
 * @brief The common C-level Interrupt Service Routine handler.
 *
 * This function is called by the assembly `isr_common_stub` for all CPU exceptions.
 * It currently prints a panic message and halts the system for any exception.
 *
 * @param regs A pointer to the `registers` structure containing the CPU state
 *             at the time of the interrupt.
 */
extern "C" void isr_handler(struct registers* regs) {
    if (regs->int_no < 32) {
        printf("CPU Exception: %d. System Halted.\n", regs->int_no);
        for (;;) {
            asm volatile("hlt");
        }
    }
}