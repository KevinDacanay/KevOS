#ifndef KERNEL_ARCH_I386_CPU_PIC_H
#define KERNEL_ARCH_I386_CPU_PIC_H

#include <stdint.h>

extern "C" {
void pic_remap(int offset1, int offset2);
void pic_eoi(uint8_t irq);
}

#endif // KERNEL_ARCH_I386_CPU_PIC_H