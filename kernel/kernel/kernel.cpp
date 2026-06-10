#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/arch/i386/mm/include/pmm.h>
#include <kernel/arch/i386/mm/include/vmm.h>
#include <kernel/arch/i386/cpu/include/pic.h>
#include <kernel/arch/i386/cpu/include/gdt.h>
#include <kernel/arch/i386/cpu/include/idt.h>
#include <kernel/arch/i386/drivers/include/keyboard.h>
#include <kernel/arch/i386/drivers/include/shell.h>

extern "C" void kernel_main(uint32_t magic, uint32_t multiboot_ptr) {
    terminal_initialize();
    gdt_install();         // Initialize the Global Descriptor Table first
    pic_remap(0x20, 0x28); // Remap PIC to start at IRQ 32 (0x20) and 40 (0x28)
    idt_install();         // Initialize and load the IDT
    keyboard_install();    // Register the keyboard IRQ handler
    
    asm volatile("sti");   // Enable hardware interrupts
    
    printf("\n");
    
    if (magic != 0x2BADB002) {
        printf("Warning: Invalid Multiboot magic number: 0x%x\n", magic);
    }
    
    multiboot_info* mbi = (multiboot_info*)multiboot_ptr;
    if (mbi->flags & (1 << 6)) { // Check if mmap is available
        pmm_init(mbi->mmap_addr, mbi->mmap_length);
        vmm_init();             // Enable Paging
    }

    printf("\n");
    printf("Hello, welcome to KevOS!\n");
    
    // Enter the interactive shell
    shell_main();
}