#ifndef KERNEL_VMM_H
#define KERNEL_VMM_H

#include <stdint.h>
#include <stddef.h>

// Page Directory Entry flags
#define VMM_PRESENT  0x1
#define VMM_WRITABLE 0x2
#define VMM_USER     0x4

extern "C" {
/**
 * @brief Initializes paging by identity mapping the first 4MB and enabling the CR0 bit.
 */
void vmm_init();

/** @brief Maps a virtual address to a physical address in the kernel directory. */
void vmm_map_page(void* phys, void* virt, uint32_t flags);
}

#endif