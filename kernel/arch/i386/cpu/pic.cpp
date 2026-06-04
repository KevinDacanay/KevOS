/**
 * @file pic.cpp
 * @brief Programmable Interrupt Controller (PIC) management.
 *
 * This file contains functions for remapping and interacting with the
 * 8259A Programmable Interrupt Controllers (PICs) in master/slave configuration.
 */

#include "pic.h"
#include "io.h" // For outb and inb
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

// Initialization Command Words (ICWs)
#define ICW1_ICW4       0x01    // ICW4 (not) present
#define ICW1_INIT       0x10    // Initialization - required!
#define ICW4_8086       0x01    // 8086/88 (MCS-80/85) mode

extern "C" {

/**
 * @brief Remaps the PICs to new interrupt vector offsets.
 *
 * By default, the PICs use IRQ vectors 0-15, which conflict with CPU exceptions.
 * This function remaps them to a safe range (e.g., 32-47).
 * @param offset1 The desired starting vector for the Master PIC (IRQs 0-7).
 * @param offset2 The desired starting vector for the Slave PIC (IRQs 8-15).
 */
void pic_remap(int offset1, int offset2) {
    uint8_t a1, a2;

    a1 = inb(PIC1_DATA); // save masks
    a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); // starts the initialization sequence (in cascade mode)
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
    io_wait();
    outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
    io_wait();
    outb(PIC1_DATA, 4); // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    io_wait();
    outb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, a1); // restore saved masks.
    outb(PIC2_DATA, a2);
}

/**
 * @brief Sends an End-Of-Interrupt (EOI) signal to the PICs.
 *
 * This must be done at the end of every IRQ handler to allow further interrupts
 * from the PIC. If the IRQ came from the slave PIC, an EOI must be sent to both.
 * @param irq The IRQ number (0-15) that was handled.
 */
void pic_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, 0x20); // EOI to slave
    }
    outb(PIC1_COMMAND, 0x20); // EOI to master
}
} // extern "C"