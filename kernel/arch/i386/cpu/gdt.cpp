/**
 * @file gdt.cpp
 * @brief Global Descriptor Table (GDT) implementation.
 *
 * This file contains the C++ implementation for setting up and installing the GDT.
 * The GDT is crucial for defining memory segments and privilege levels in Protected Mode.
 */

#include <kernel/arch/i386/cpu/include/gdt.h>

// Array of GDT entries. We need at least 3: a null segment, a code segment, and a data segment.
struct gdt_entry_struct gdt_entries[3];
// Pointer structure used by the `lgdt` instruction to load the GDT.
struct gdt_ptr_struct gdt_ptr;

/**
 * @brief Sets up a single GDT entry.
 *
 * This function populates a GDT entry with the specified base address, limit,
 * access flags, and granularity.
 *
 * @param num The index of the GDT entry to set.
 * @param base The 32-bit base address of the segment.
 * @param limit The 20-bit limit of the segment (in 1-byte or 4KB units).
 * @param access The access byte, defining type, privilege, and presence.
 * @param gran The granularity byte, defining size (16/32-bit) and limit units (byte/4KB).
 */
extern "C" void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F; // Limit high 4 bits

    gdt_entries[num].granularity |= gran & 0xF0; // Granularity flags (D/B, G)
    gdt_entries[num].access      = access;       // Access byte
}

/**
 * @brief Initializes and loads the GDT.
 *
 * This function sets up the GDT pointer, creates the necessary segment descriptors
 * (null, kernel code, kernel data), and then loads the GDT into the CPU using `lgdt`.
 */
extern "C" void gdt_install() {
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 3) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                       // Null segment (required)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);        // Kernel Code segment (base 0, limit 4GB, 32-bit, ring 0)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);        // Kernel Data segment (base 0, limit 4GB, 32-bit, ring 0)

    gdt_flush((uint32_t)&gdt_ptr);
}