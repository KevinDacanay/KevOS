/**
 * @file irq.cpp
 * @brief Interrupt Request (IRQ) handler management.
 *
 * This file provides the C-level infrastructure for registering and dispatching
 * custom handlers for hardware interrupts (IRQs). It also includes the common
 * IRQ handler called by the assembly stubs.
 */

#include <kernel/arch/i386/cpu/include/irq.h>
#include <kernel/arch/i386/cpu/include/pic.h>
#include <kernel/include/kernel/task.h>
#include <stddef.h>

// Global tick counter for the system timer (IRQ 0)
static uint32_t timer_ticks = 0;

// Array of function pointers for custom IRQ handlers.
// Each element corresponds to an IRQ line (0-15).
void* irq_routines[16] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/**
 * @brief Installs a custom handler for a specific IRQ line.
 *
 * @param irq The IRQ number (0-15) to install the handler for.
 * @param handler A function pointer to the custom handler. This function
 *                will receive a `registers` struct as its argument.
 */
extern "C" void irq_install_handler(int irq, void (*handler)(struct registers *r)) {
    irq_routines[irq] = (void*)handler;
}

/**
 * @brief The common C-level IRQ handler.
 *
 * This function is called by the assembly `irq_common_stub`. It dispatches
 * to the registered custom handler for the specific IRQ and then sends
 * an End-Of-Interrupt (EOI) signal to the PICs.
 *
 * @param regs A pointer to the `registers` structure containing the CPU state
 *             at the time of the interrupt.
 */
extern "C" void irq_handler(struct registers* regs) {
    void (*handler)(struct registers *r);

    // IRQ 0 (the system timer) is mapped to interrupt 32.
    if (regs->int_no == 32) {
        timer_ticks++;
    }

    handler = (void (*)(struct registers *r))irq_routines[regs->int_no - 32];
    if (handler) {
        handler(regs);
    }
    
    // If this was a timer interrupt, trigger the scheduler for preemption
    if (regs->int_no == 32) {
        pic_eoi(0); // Must send EOI before switching tasks
        schedule();
        return;
    }

    pic_eoi(regs->int_no - 32); // Send EOI to the appropriate PIC (master or slave)
}

/**
 * @brief Returns the total number of system ticks since the PIT was initialized.
 * 
 * @return uint32_t The current tick count.
 */
extern "C" uint32_t get_timer_ticks() {
    return timer_ticks;
}