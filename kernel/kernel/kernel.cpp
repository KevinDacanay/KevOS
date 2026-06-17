/**
 * @file kernel.cpp
 * @brief Architecture-independent kernel initialization entry point.
 * 
 * orchestrates the setup of core CPU structures (GDT, IDT) and 
 * memory management (PMM, VMM, Heap) before launching the user interface.
 */
#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/arch/i386/mm/include/pmm.h>
#include <kernel/arch/i386/mm/include/vmm.h>
#include <kernel/arch/i386/cpu/include/pit.h>
#include <kernel/arch/i386/mm/include/heap.h>
#include <kernel/arch/i386/cpu/include/pic.h>
#include <kernel/arch/i386/cpu/include/gdt.h>
#include <kernel/arch/i386/cpu/include/idt.h>
#include <kernel/arch/i386/cpu/include/irq.h>
#include <kernel/arch/i386/drivers/include/keyboard.h>
#include <kernel/include/kernel/syscall.h>
#include <kernel/arch/i386/drivers/include/shell.h>
#include <kernel/include/kernel/task.h>

/**
 * @brief Assembly helper to jump to Ring 3.
 */
extern "C" void jump_to_usermode(uint32_t entry, uint32_t user_stack);

/**
 * @brief A simple task to test multitasking.
 * This will now run in Ring 3!
 */
void test_task() {
    uint32_t last_tick = 0;
    const char* msg = "[Task 2 Heartbeat via Syscall]\n";

    while (true) {
        uint32_t current_tick;
        // Syscall 1: Get Ticks
        asm volatile("mov $1, %%eax; int $0x80" : "=a"(current_tick));

        if (current_tick - last_tick >= 200) { // Print roughly every 2 seconds
            // Syscall 0: Printf
            asm volatile("mov $0, %%eax; mov %0, %%ebx; int $0x80" : : "r"(msg) : "eax", "ebx");
            last_tick = current_tick;
        }
    }
}

void user_mode_proxy() {
    // Allocate a small stack for user mode in the heap
    uint32_t user_stack = (uint32_t)kmalloc(4096) + 4096;
    jump_to_usermode((uint32_t)test_task, user_stack);
}

/**
 * @brief The main kernel entry point.
 * 
 * Called by boot.S after the stack and basic paging are set up.
 * @param magic Multiboot magic number (should be 0x2BADB002).
 * @param multiboot_ptr Physical pointer to the multiboot info structure.
 */
extern "C" void kernel_main(uint32_t magic, uint32_t multiboot_ptr) {
    terminal_initialize();
    gdt_install();         // Initialize the Global Descriptor Table first
    
    // The stack_top symbol is defined in boot.S and represents the top of the kernel stack.
    extern uint32_t stack_top; 
    tss_install((uint32_t)&stack_top); // Initialize TSS with the kernel stack
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
        kheap_init();           // Initialize Kernel Heap
        pit_init(100);          // Initialize PIT to 100 Hz
        syscall_init();         // Initialize System Calls
    }

    tasking_init();
    task_create(user_mode_proxy);

    printf("\n");
    printf("Hello, welcome to KevOS!\n");
    
    // Enter the interactive shell
    shell_main();
}