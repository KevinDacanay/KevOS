#ifndef KERNEL_PIT_H
#define KERNEL_PIT_H

#include <stdint.h>

// PIT Ports
#define PIT_CHANNEL0_DATA 0x40 // Channel 0 data port (read/write)
#define PIT_CHANNEL1_DATA 0x41 // Channel 1 data port (read/write)
#define PIT_CHANNEL2_DATA 0x42 // Channel 2 data port (read/write)
#define PIT_COMMAND_PORT  0x43 // Command register (write only)

extern "C" {
/**
 * @brief Initializes the Programmable Interval Timer (PIT).
 * @param frequency The desired frequency in Hz (e.g., 100 for 100Hz).
 */
void pit_init(uint32_t frequency);
}

#endif // KERNEL_PIT_H