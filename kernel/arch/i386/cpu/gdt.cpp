/**
 * @file gdt.cpp
 * @brief Global Descriptor Table (GDT) implementation.
 *
 * This file contains the C++ implementation for setting up and installing the GDT.
 * The GDT is crucial for defining memory segments and privilege levels in Protected Mode.
 */
#include <kernel/arch/i386/cpu/include/gdt.h>
#include <string.h>
#include <stdio.h>

// Array of GDT entries. 
// We need 6: Null, KCode, KData, UCode, UData, and TSS.
#define GDT_ENTRIES 6
struct gdt_entry_struct gdt_entries[GDT_ENTRIES];

static struct tss_entry_struct tss_entry;

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
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * GDT_ENTRIES) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel Code
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel Data
    
    // User Mode segments (DPL 3)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User Code
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User Data

    // TSS segment: Note the access byte 0xE9 (Present, DPL 3, Type: Available 386 TSS)
    uint32_t tss_base = (uint32_t)&tss_entry;
    uint32_t tss_limit = sizeof(struct tss_entry_struct) - 1;
    gdt_set_gate(5, tss_base, tss_limit, 0xE9, 0x00);

    gdt_flush((uint32_t)&gdt_ptr);
}

/**
 * @brief Sets up the Task State Segment (TSS) with the kernel stack.
 */
extern "C" void tss_install(uint32_t kernel_stack_ptr) {
    // Clear the TSS
    memset(&tss_entry, 0, sizeof(struct tss_entry_struct));

    // Set the kernel stack that the CPU should switch to when an interrupt occurs in Ring 3
    tss_entry.ss0 = GDT_KERNEL_DATA_SEGMENT;
    tss_entry.esp0 = kernel_stack_ptr;

    // Set the segment registers for the TSS
    // These aren't strictly used for software context switching, but good for completeness
    tss_entry.cs = GDT_KERNEL_CODE_SEGMENT | 0x03; // DPL 3
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = GDT_KERNEL_DATA_SEGMENT | 0x03;

    // Load the TSS descriptor index into the Task Register (TR)
    // The selector is 0x28 (index 5)
    asm volatile("ltr %%ax" : : "a" (GDT_TSS_SEGMENT));

    printf("TSS installed. Kernel stack top: 0x%x\n", kernel_stack_ptr);
}