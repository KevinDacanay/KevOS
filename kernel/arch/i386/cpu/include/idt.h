/**
 * @file idt.h
 * @brief Interrupt Descriptor Table (IDT) definitions and management.
 *
 * The IDT is an x86-specific data structure used to implement an interrupt vector table.
 * It provides the CPU with the entry points (Interrupt Service Routines) for:
 * 1. CPU Exceptions (e.g., Page Faults, Division by Zero)
 * 2. Hardware Interrupts (IRQs from PIC/APIC)
 * 3. Software Interrupts (System Calls)
 */

#ifndef KERNEL_ARCH_I386_CPU_IDT_H
#define KERNEL_ARCH_I386_CPU_IDT_H

#include <stdint.h>

/**
 * @struct idt_entry_struct
 * @brief Represents a single gate/entry in the IDT.
 * 
 * This 8-byte structure tells the CPU what segment to switch to and the offset of the handler.
 */
struct idt_entry_struct {
    uint16_t base_low;      // The lower 16 bits of the address to jump to when this interrupt fires.
    uint16_t sel;           // Kernel segment selector.
    uint8_t  always0;       // This must always be zero.
    uint8_t  flags;         // Flags. See documentation.
    uint16_t base_high;     // The upper 16 bits of the address to jump to.
} __attribute__((packed));

/**
 * @struct idt_ptr_struct
 * @brief The pointer structure passed to the 'lidt' assembly instruction.
 * 
 * Contains the limit (size of the table - 1) and the linear base address of the IDT array.
 */
struct idt_ptr_struct {
    uint16_t limit;         // The size of the IDT table in bytes minus 1.
    uint32_t base;          // The address of the first element in our idt_entry_t array.
} __attribute__((packed));

/**
 * @struct registers
 * @brief Represents the CPU state (register values) saved on the stack during an interrupt.
 * 
 * This structure matches the stack layout created by the common interrupt stub, including:
 * - Segments pushed manually
 * - General purpose registers pushed by 'pusha'
 * - Interrupt number and error code pushed by the specific ISR stub
 * - Processor-pushed values (EIP, CS, EFLAGS, etc.)
 */
struct registers {
    uint32_t ds;                                    // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;                      // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss;          // Pushed by the processor automatically during context switch
};

extern "C" {
/**
 * @brief Sets a gate in the IDT.
 * @param num The interrupt vector number (0-255).
 * @param base The address of the ISR handler.
 * @param sel The kernel code segment selector.
 * @param flags Gate type and attributes (e.g., Presence, Privilege Level).
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

/** @brief Loads the IDT into the CPU and initializes all gates to a default state. */
void idt_install(void);
}

#endif // KERNEL_ARCH_I386_CPU_IDT_H