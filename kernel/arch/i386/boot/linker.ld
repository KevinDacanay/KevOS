/*
 * Linker script for the i686-elf kernel.
 * This script defines the memory layout of the kernel image.
 */

ENTRY(_start) /* The kernel entry point is the _start symbol from boot.S */

/* Define the virtual and physical offsets */
VIRT_BASE = 0xC0000000;
PHYS_BASE = 0x100000;

SECTIONS
{
    /* The kernel starts at 1MB physically, but symbols start at 3GB + 1MB */
    . = VIRT_BASE + PHYS_BASE;
    _kernel_start = .;

    /* Use AT() to specify the Load Memory Address (LMA) */
    
    .text : AT(ADDR(.text) - VIRT_BASE) ALIGN(0x1000)
    {
        *(.multiboot) /* The Multiboot header MUST be the first thing in the kernel. */
        *(.text)      /* All other code follows. */
    }

    .data : AT(ADDR(.data) - VIRT_BASE) ALIGN(0x1000)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - VIRT_BASE) ALIGN(0x1000)
    {
        *(.bss)
        *(COMMON)
    }

    _kernel_end = .;
}