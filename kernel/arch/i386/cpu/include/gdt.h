/**
 * @file gdt.h
 * @brief Global Descriptor Table (GDT) definitions.
 *
 * This file defines the structures for GDT entries and the GDT pointer,
 * along with function prototypes for GDT management.
 * The GDT is essential for memory segmentation and privilege levels in x86 Protected Mode.
 */

#ifndef KERNEL_ARCH_I386_CPU_GDT_H
#define KERNEL_ARCH_I386_CPU_GDT_H

#include <stdint.h>

// Segment Selectors (offsets into the GDT)
#define GDT_KERNEL_CODE_SEGMENT 0x08 ///< Index 1 * 8
#define GDT_KERNEL_DATA_SEGMENT 0x10 ///< Index 2 * 8
#define GDT_USER_CODE_SEGMENT   0x18 ///< Index 3 * 8
#define GDT_USER_DATA_SEGMENT   0x20 ///< Index 4 * 8

// TSS Selector (Index 5 * 8 = 40 or 0x28)
#define GDT_TSS_SEGMENT         0x28 

/**
 * @struct gdt_entry_struct
 * @brief Represents a single 8-byte entry in the GDT.
 *
 * Defines a memory segment's base address, limit, and access rights.
 */
struct gdt_entry_struct {
    uint16_t limit_low;     ///< The lower 16 bits of the segment limit.
    uint16_t base_low;      ///< The lower 16 bits of the segment base address.
    uint8_t  base_middle;   ///< The middle 8 bits of the segment base address.
    uint8_t  access;        ///< Access byte (type, S, DPL, P flags).
    uint8_t  granularity;   ///< Granularity byte (limit high 4 bits, AVL, L, D/B, G flags).
    uint8_t  base_high;     ///< The upper 8 bits of the segment base address.
} __attribute__((packed));

/**
 * @struct gdt_ptr_struct
 * @brief Pointer structure for the `lgdt` instruction.
 *
 * Contains the size of the GDT and its linear base address.
 */
struct gdt_ptr_struct {
    uint16_t limit;         ///< The size of the GDT in bytes minus 1.
    uint32_t base;          ///< The linear address of the GDT.
} __attribute__((packed));

/**
 * @brief Structure representing the Task State Segment (TSS).
 * 
 * Required for privilege level transitions (e.g., from Ring 3 to Ring 0).
 */
struct tss_entry_struct {
    uint32_t prev_tss;   // The previous TSS - if we're doing hardware task switching
    uint32_t esp0;       // The stack pointer to load when changing to kernel mode
    uint32_t ss0;        // The stack segment to load when changing to kernel mode
    uint32_t esp1;       // Unused
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

extern "C" {
void gdt_install();                                                                 ///< @brief Initializes and loads the GDT.
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran); ///< @brief Sets up a single GDT entry.
extern void gdt_flush(uint32_t);                                                    ///< @brief Assembly function to load the GDT and reload segment registers.

/**
 * @brief Sets up the Task State Segment (TSS) with the kernel stack.
 * @param kernel_stack_ptr The virtual address of the kernel stack to use for Ring 0.
 */
void tss_install(uint32_t kernel_stack_ptr);
}

#endif