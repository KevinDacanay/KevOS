#ifndef KERNEL_PMM_H
#define KERNEL_PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

extern "C" {

/**
 * @brief Initializes the Physical Memory Manager.
 * @param mmap_addr Pointer to the Multiboot memory map.
 * @param mem_size Total size of memory in bytes.
 */
void pmm_init(uint32_t mmap_addr, uint32_t mmap_length);

void* pmm_alloc_page();
void pmm_free_page(void* ptr);

size_t pmm_get_free_memory();
}

#endif