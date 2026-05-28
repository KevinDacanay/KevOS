#ifndef KERNEL_ARCH_I386_CPU_IDT_H
#define KERNEL_ARCH_I386_CPU_IDT_H

#include <stdint.h>

// Define an IDT entry structure
struct idt_entry_struct {
    uint16_t base_low;      // The lower 16 bits of the address to jump to when this interrupt fires.
    uint16_t sel;           // Kernel segment selector.
    uint8_t  always0;       // This must always be zero.
    uint8_t  flags;         // Flags. See documentation.
    uint16_t base_high;     // The upper 16 bits of the address to jump to.
} __attribute__((packed));

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format usable by the 'lidt' instruction.
struct idt_ptr_struct {
    uint16_t limit;         // The upper 16 bits of the address to jump to.
    uint32_t base;          // The address of the first element in our idt_entry_t array.
} __attribute__((packed));

// A structure to hold the CPU registers when an interrupt occurs.
// This is pushed onto the stack by the assembly interrupt stub.
struct registers {
    uint32_t ds;                                    // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;                      // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss;          // Pushed by the processor automatically
};

extern "C" {
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_install(void);
}

#endif // KERNEL_ARCH_I386_CPU_IDT_H