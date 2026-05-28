#ifndef KERNEL_ARCH_I386_CPU_GDT_H
#define KERNEL_ARCH_I386_CPU_GDT_H

#include <stdint.h>

struct gdt_entry_struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern "C" {
void gdt_install();
void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
// This is defined in assembly to load the GDT
extern void gdt_flush(uint32_t);
}

#endif