/**
 * @file interrupts.S
 * @brief Assembly stubs for Interrupt Service Routines (ISRs) and Interrupt Request (IRQ) handlers.
 *
 * This file defines the low-level assembly entry points for CPU exceptions (ISRs)
 * and hardware interrupts (IRQs). It also includes the `gdt_flush` routine
 * for loading the GDT and reloading segment registers.
 */

// Macro for Interrupt Service Routines that do NOT push an error code.
// These exceptions (e.g., Divide-by-zero) do not automatically push an error code
// onto the stack, so we push a dummy 0 to maintain a consistent stack frame
// for the common handler.
.macro ISR_NOERRCODE num
  .global isr\num
  isr\num:
    pushl $0            // Push a dummy error code (0)
    pushl $\num         // Push the interrupt number
    jmp isr_common_stub // Jump to the common ISR handler
.endm

// Macro for Interrupt Service Routines that DO push an error code.
// These exceptions (e.g., Page Fault) automatically push an error code,
// so we only need to push the interrupt number.
.macro ISR_ERRCODE num
  .global isr\num
  isr\num:
    pushl $\num         // Push the interrupt number (error code is already on stack)
    jmp isr_common_stub // Jump to the common ISR handler
.endm

// Macro for Interrupt Request (IRQ) handlers.
// IRQs do not push an error code, so we push a dummy 0.
// The 'target' parameter is the remapped IRQ number (e.g., 32 for IRQ0).
.macro IRQ num, target
  .global irq\num
  irq\num:
    pushl $0            // Push a dummy error code (0)
    pushl $\target      // Push the remapped IRQ number
    jmp irq_common_stub // Jump to the common IRQ handler
.endm

.global gdt_flush
gdt_flush:
    mov 4(%esp), %eax
    lgdt (%eax)
    mov $0x10, %ax
    mov %ax, %ds        // Reload data segment register
    mov %ax, %es        // Reload extra segment register
    mov %ax, %fs        // Reload FS segment register
    mov %ax, %gs        // Reload GS segment register
    ljmp $0x08, $.flush // Far jump to reload CS register (0x08 is kernel code segment selector)
.flush:
    ret

// Define all 32 CPU exceptions.
// Some push error codes automatically (e.g., 8, 10-14, 17), others do not.
// The macros handle pushing a dummy error code for those that don't,
// ensuring a consistent stack frame for `isr_common_stub`.
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

// Define all 16 hardware IRQs.
// These are remapped to interrupt vectors 32-47 by the PIC.
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

.extern isr_handler
// Common stub for all Interrupt Service Routines (CPU exceptions).
// This stub saves the CPU state, sets up kernel data segments, calls the
// C-level `isr_handler`, restores the CPU state, and returns from interrupt.
isr_common_stub:
    pusha                   // Push all general-purpose registers (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI)
    mov %ds, %ax            // Save current data segment selector
    push %eax               // Push it onto the stack
    mov $0x10, %ax          // Load kernel data segment selector (0x10)
    mov %ax, %ds            // Set DS, ES, FS, GS to kernel data segment
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    pushl %esp              // Push the address of the stack frame (containing registers)
    call isr_handler        // Call the C-level ISR handler
    add $4, %esp            // Pop the pushed ESP
    pop %eax                // Restore original data segment selector
    mov %ax, %ds            // Restore DS, ES, FS, GS
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    popa                    // Pop all general-purpose registers
    add $8, %esp            // Pop the interrupt number and error code (pushed by ISR_NOERRCODE/ISR_ERRCODE)
    iret                    // Return from interrupt

.extern irq_handler
// Common stub for all Interrupt Request (IRQ) handlers.
// Similar to `isr_common_stub`, but calls `irq_handler` and handles EOI if necessary.
irq_common_stub:
    pusha
    mov %ds, %ax
    push %eax
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    pushl %esp
    call irq_handler        // Call the C-level IRQ handler
    add $4, %esp
    pop %ebx                // Restore original data segment selector
    mov %bx, %ds            // Restore DS, ES, FS, GS
    mov %bx, %es
    mov %bx, %fs
    mov %bx, %gs
    popa
    add $8, %esp
    iret