#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/arch/i386/cpu/pic.h>
#include <kernel/arch/i386/cpu/gdt.h>
#include <kernel/arch/i386/cpu/idt.h>
#include <kernel/arch/i386/drivers/keyboard.h>

extern "C" void kernel_main() {
    terminal_initialize();
    gdt_install();         // Initialize the Global Descriptor Table first
    pic_remap(0x20, 0x28); // Remap PIC to start at IRQ 32 (0x20) and 40 (0x28)
    idt_install();         // Initialize and load the IDT
    keyboard_install();    // Register the keyboard IRQ handler

    asm volatile("sti");   // Enable hardware interrupts

    printf("Hello, kernel!\n");

    // Keep the kernel alive so interrupts can continue to fire
    for (;;) { }
}