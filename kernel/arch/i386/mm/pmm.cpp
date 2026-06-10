/**
 * @file pmm.cpp
 * @brief Physical Memory Manager (PMM) implementation for KevOS.
 *
 * This file provides the core functionality for managing physical memory pages.
 * It uses a bitmap to track the allocation status of 4KB physical pages.
 * The PMM is responsible for initializing the bitmap based on the Multiboot
 * memory map, marking kernel and reserved areas as used, and providing
 * functions to allocate and free physical pages.
 */

#include <kernel/arch/i386/mm/include/pmm.h>
#include <kernel/arch/i386/mm/include/vmm.h>
#include <kernel/multiboot.h>
#include <string.h>
#include <stdio.h>

// These symbols are defined in the linker script to mark the start and end
// of the kernel's loaded image in physical memory.
extern "C" char _kernel_start[];
extern "C" char _kernel_end[];

// Pointer to the physical memory bitmap. Each bit represents a 4KB page.
// A set bit (1) means the page is used, a clear bit (0) means it's free.
static uint8_t* pmm_bitmap = nullptr; 

// Total number of physical pages (blocks) managed by the PMM.
static size_t pmm_max_blocks = 0; 

// Number of currently used physical pages.
static size_t pmm_used_blocks = 0; 

// The size of a physical page in bytes. Defined in pmm.h.
// For i386, this is typically 4096 bytes (4KB).

// Helper functions for bit manipulation
/**
 * @brief Marks a specific physical page as used in the bitmap.
 * @param bit The index of the page to mark as used.
 */
static inline void mmap_set(uint32_t bit) {
    pmm_bitmap[bit / 8] |= (1 << (bit % 8));
}

/**
 * @brief Marks a specific physical page as free in the bitmap.
 * @param bit The index of the page to mark as free.
 */
static inline void mmap_unset(uint32_t bit) {
    pmm_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

/**
 * @brief Checks if a specific physical page is used.
 * @param bit The index of the page to check.
 * @return True if the page is used, false otherwise.
 */
static inline bool mmap_test(uint32_t bit) {
    return (pmm_bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}

/**
 * @brief Initializes the Physical Memory Manager.
 *
 * This function parses the Multiboot memory map provided by the bootloader
 * to determine the available physical memory. It then sets up a bitmap
 * to track page allocation, marks reserved areas (like the kernel and the
 * bitmap itself) as used, and frees up available memory regions.
 *
 * @param mmap_addr The physical address of the Multiboot memory map structure.
 * @param mmap_length The total length of the Multiboot memory map in bytes.
 */
void pmm_init(uint32_t mmap_addr, uint32_t mmap_length) {
    // 1. Determine the highest physical address to size the bitmap.
    //    This ensures the bitmap covers all potentially usable physical memory.
    uint64_t highest_physical_addr = 0;
    multiboot_mmap_entry* entry = (multiboot_mmap_entry*)mmap_addr;
    while ((uint32_t)entry < mmap_addr + mmap_length) {
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) { // Only consider available memory regions
            uint64_t current_end = (uint64_t)entry->addr + entry->len; // Cast to uint64_t to prevent overflow
            if (current_end > highest_physical_addr) {
                highest_physical_addr = current_end;
            }
        }
        entry = (multiboot_mmap_entry*)((uint32_t)entry + entry->size + sizeof(entry->size));
    }

    // Calculate the total number of 4KB pages up to the highest physical address.
    pmm_max_blocks = (size_t)(highest_physical_addr / PAGE_SIZE);
    
    // 2. Place the bitmap immediately after the kernel in memory.
    //    The kernel's end address is provided by the linker script.
    //    We subtract KERNEL_VIRT_BASE because the bitmap is stored in physical RAM.
    //    Note: We cast to uint32_t first to handle the pointer arithmetic correctly.
    uint32_t phys_kernel_end = (uint32_t)_kernel_end - KERNEL_VIRT_BASE;
    pmm_bitmap = (uint8_t*)phys_kernel_end;
    
    // Calculate the size of the bitmap in bytes.
    // Each bit represents a page, so (total pages / 8) bytes are needed.
    size_t bitmap_size = pmm_max_blocks / 8;
    if (pmm_max_blocks % 8) { // Account for any partial last byte
        bitmap_size++;
    }

    // Initially mark all pages as used (reserved).
    // This is a safe default; we will free available regions next.
    memset(pmm_bitmap, 0xFF, bitmap_size);
    
    // Initialize pmm_used_blocks to reflect all pages being marked as used.
    pmm_used_blocks = pmm_max_blocks;

    // 3. Parse the Multiboot memory map again to free available regions.
    //    Iterate through each entry and mark 'MULTIBOOT_MEMORY_AVAILABLE' regions as free.
    entry = (multiboot_mmap_entry*)mmap_addr;
    while ((uint32_t)entry < mmap_addr + mmap_length) {
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint32_t block_start = (uint32_t)(entry->addr / PAGE_SIZE);
            uint32_t block_count = (uint32_t)(entry->len / PAGE_SIZE);
            
            for (uint32_t i = 0; i < block_count; i++) {
                // Ensure we don't try to unset bits beyond our bitmap's capacity
                if ((block_start + i) < pmm_max_blocks) {
                    if (mmap_test(block_start + i)) { // Only decrement if it was previously set
                        mmap_unset(block_start + i);
                        pmm_used_blocks--;
                    }
                }
            }
        }
        entry = (multiboot_mmap_entry*)((uint32_t)entry + entry->size + sizeof(entry->size));
    }

    // 4. Important: Re-lock (mark as used) the pages occupied by the kernel
    //    and the PMM's own bitmap structure.
    //    This prevents the PMM from allocating memory that is already in use
    //    by the kernel or its own management structures.
    //    We convert virtual symbols to physical addresses.
    uint32_t phys_kernel_start = (uint32_t)_kernel_start - KERNEL_VIRT_BASE;
    uint32_t kernel_start_block = phys_kernel_start / PAGE_SIZE;

    // The end of the reserved area includes the kernel and the bitmap itself.
    // We round up to the next page boundary for safety.
    uint32_t reserved_end_addr = phys_kernel_end + bitmap_size;
    uint32_t kernel_and_bitmap_end_block = (reserved_end_addr + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = kernel_start_block; i < kernel_and_bitmap_end_block; i++) {
        if (i < pmm_max_blocks) { // Ensure we don't go out of bounds
            if (!mmap_test(i)) { // Only increment if it was previously free
                mmap_set(i);
                pmm_used_blocks++;
            }
        }
    }

    printf("PMM initialized with %d MB total physical memory.\n", (uint32_t)(highest_physical_addr / 1024 / 1024));
    printf("Bitmap size: %d bytes at 0x%x\n", bitmap_size, (uint32_t)pmm_bitmap);
    printf("Free memory: %d KB\n", pmm_get_free_memory() / 1024);
}

/**
 * @brief Returns the amount of free physical memory in bytes.
 * @return The total number of free bytes.
 */
size_t pmm_get_free_memory() {
    return (pmm_max_blocks - pmm_used_blocks) * PAGE_SIZE;
}

/**
 * @brief Allocates a single free physical page.
 *
 * Searches the bitmap for the first available (free) page, marks it as used,
 * and returns its physical address.
 *
 * @return The physical address of the allocated page, or nullptr if no free page is found.
 */
void* pmm_alloc_page() {
    // Find the first free bit (page) in the bitmap
    for (size_t i = 0; i < pmm_max_blocks; i++) {
        if (!mmap_test(i)) { // If the bit is 0 (page is free)
            mmap_set(i);     // Mark it as used
            pmm_used_blocks++;
            return (void*)(i * PAGE_SIZE); // Return the physical address of the page
        }
    }
    return nullptr; // No free pages found
}

/**
 * @brief Frees a previously allocated physical page.
 *
 * Marks the page corresponding to the given physical address as free in the bitmap.
 *
 * @param ptr The physical address of the page to free.
 */
void pmm_free_page(void* ptr) {
    uint32_t page_index = (uint32_t)ptr / PAGE_SIZE;
    if (page_index < pmm_max_blocks && mmap_test(page_index)) { // Ensure it's a valid and currently used page
        mmap_unset(page_index); // Mark it as free
        pmm_used_blocks--;
    }
}