/*
 * Linker script for the i686-elf kernel.
 * This script defines the memory layout of the kernel image.
 */

ENTRY(_start) /* The kernel entry point is the _start symbol from boot.S */

SECTIONS
{
    /* The kernel will be loaded by GRUB at 1MB (0x100000). */
    . = 0x100000;

    /* The .text section contains executable code. */
    .text : ALIGN(0x1000)
    {
        *(.multiboot) /* The Multiboot header MUST be the first thing in the kernel. */
        *(.text)      /* All other code follows. */
    }

    /* The .data section contains initialized data. */
    .data : ALIGN(0x1000) { *(.data) }

    /* The .bss section contains uninitialized data. */
    .bss : ALIGN(0x1000) { *(.bss) }
}