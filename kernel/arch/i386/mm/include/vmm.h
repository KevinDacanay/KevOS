#ifndef KERNEL_VMM_H
#define KERNEL_VMM_H

#include <stdint.h>
#include <stddef.h>

// Page Directory Entry flags
#define VMM_PRESENT  0x1
#define VMM_WRITABLE 0x2
#define VMM_USER     0x4

// The virtual address where the kernel is mapped (3GB)
#define KERNEL_VIRT_BASE 0xC0000000

// Recursive mapping constants
// The 1023rd entry of the PD points to the PD itself.
#define VMM_RECURSIVE_SLOT 1023
#define VMM_PAGE_TABLES    0xFFC00000
#define VMM_PAGE_DIRECTORY 0xFFFFF000

extern "C" {
/**
 * @brief Initializes paging by identity mapping the first 4MB and enabling the CR0 bit.
 */
void vmm_init();

/** @brief Maps a virtual address to a physical address in the kernel directory. */
void vmm_map_page(void* phys, void* virt, uint32_t flags);
}

#endif