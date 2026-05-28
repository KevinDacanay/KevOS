#ifndef KERNEL_ARCH_I386_CPU_IRQ_H
#define KERNEL_ARCH_I386_CPU_IRQ_H

#include "idt.h"

extern "C" {
void irq_install_handler(int irq, void (*handler)(struct registers *r));
}

#endif