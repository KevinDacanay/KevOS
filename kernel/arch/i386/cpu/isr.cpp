/**
 * @file isr.cpp
 * @brief Interrupt Service Routine (ISR) handler for CPU exceptions.
 *
 * This file contains the common C-level handler for CPU exceptions (ISRs 0-31).
 * It is called by the assembly `isr_common_stub` after saving the CPU state.
 */

#include <kernel/arch/i386/cpu/include/idt.h>
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
        printf("\n--- KERNEL PANIC: CPU EXCEPTION ---\n");
        printf("Exception ID: %d\n", regs->int_no);
        printf("Error Code:   %d\n", regs->err_code);
        printf("EIP: 0x%x  CS: 0x%x  EFLAGS: 0x%x\n", regs->eip, regs->cs, regs->eflags);
        printf("EAX: 0x%x  EBX: 0x%x  ECX: 0x%x  EDX: 0x%x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
        printf("ESP: 0x%x  EBP: 0x%x  ESI:   0x%x  EDI: 0x%x\n", regs->esp, regs->ebp, regs->esi, regs->edi);
        printf("\nSystem Halted.\n");
        for (;;) {
            asm volatile("hlt");
        }
    }
}