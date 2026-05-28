#include "irq.h"
#include "pic.h"
#include <stddef.h>

void* irq_routines[16] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

extern "C" void irq_install_handler(int irq, void (*handler)(struct registers *r)) {
    irq_routines[irq] = (void*)handler;
}

extern "C" void irq_handler(struct registers* regs) {
    void (*handler)(struct registers *r);

    handler = (void (*)(struct registers *r))irq_routines[regs->int_no - 32];
    if (handler) {
        handler(regs);
    }

    // Send EOI to PIC
    pic_eoi(regs->int_no - 32);
}