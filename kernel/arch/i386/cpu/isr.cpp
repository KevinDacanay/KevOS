#include "idt.h"
#include <stdio.h>

extern "C" void isr_handler(struct registers* regs) {
    if (regs->int_no < 32) {
        printf("CPU Exception: %d. System Halted.\n", regs->int_no);
        for (;;) {
            asm volatile("hlt");
        }
    }
}