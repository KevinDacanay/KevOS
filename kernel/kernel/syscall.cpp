#include <kernel/include/kernel/syscall.h>
#include <stdio.h>
#include <kernel/arch/i386/cpu/include/irq.h>

void syscall_init() {
    printf("System call interface initialized (int 0x80).\n");
}

static void sys_printf(const char* str) {
    printf("%s", str); // printf returns int, but we ignore it here.
}

/**
 * @brief Array of syscall function pointers.
 */
static void* syscalls[] = {
    (void*)sys_printf,
};

#define NUM_SYSCALLS (sizeof(syscalls) / sizeof(void*))

extern "C" void syscall_handler(struct registers* regs) {
    // The syscall number is in EAX
    if (regs->eax >= NUM_SYSCALLS) {
        return;
    }

    // Dispatch based on syscall number
    switch (regs->eax) {
        case 0: { // sys_printf
            // The string pointer is expected in EBX
            typedef void (*sys_printf_func)(const char*);
            sys_printf_func func = (sys_printf_func)syscalls[0];
            func((const char*)regs->ebx);
            regs->eax = 0; // Return 0 for success
            break;
        }
        case 1: { // sys_get_ticks
            regs->eax = get_timer_ticks();
            break;
        }
        default:
            regs->eax = -1; // Indicate error for unknown syscall
    }
}