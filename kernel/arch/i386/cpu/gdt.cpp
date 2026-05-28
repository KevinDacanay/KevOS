#include "gdt.h"

struct gdt_entry_struct gdt_entries[3];
struct gdt_ptr_struct gdt_ptr;

extern "C" void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

extern "C" void gdt_install() {
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 3) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    // Null segment
    gdt_set_gate(0, 0, 0, 0, 0);
    
    // Code segment: base 0, limit 4GB, type 0x9A (present, ring 0, code, exec/read)
    // Granularity 0xCF (4KB blocks, 32-bit mode)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Data segment: base 0, limit 4GB, type 0x92 (present, ring 0, data, read/write)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_flush((uint32_t)&gdt_ptr);
}