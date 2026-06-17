#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <stdint.h>
#include <kernel/arch/i386/cpu/include/idt.h>

enum syscall_nos {
    SYS_PRINTF = 0,
    SYS_GET_TICKS = 1,
};

/**
 * @brief Initializes the system call interface.
 */
void syscall_init();

extern "C" void syscall_handler(struct registers* regs);

#endif