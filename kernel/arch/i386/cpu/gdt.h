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

extern "C" {
void gdt_install();                                                                 ///< @brief Initializes and loads the GDT.
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran); ///< @brief Sets up a single GDT entry.
extern void gdt_flush(uint32_t);                                                    ///< @brief Assembly function to load the GDT and reload segment registers.
}

#endif