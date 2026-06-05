#include <kernel/arch/i386/mm/include/pmm.h>
#include <kernel/multiboot.h>
#include <string.h>
#include <stdio.h>

extern "C" char _kernel_start[];
extern "C" char _kernel_end[];

static uint8_t* pmm_bitmap = nullptr;
static size_t pmm_max_blocks = 0;
static size_t pmm_used_blocks = 0;

// Helper functions for bit manipulation
static inline void mmap_set(uint32_t bit) {
    pmm_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void mmap_unset(uint32_t bit) {
    pmm_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

void pmm_init(uint32_t mmap_addr, uint32_t mmap_length) {
    // 1. Calculate total memory to determine bitmap size
    uint64_t total_mem = 0;
    multiboot_mmap_entry* entry = (multiboot_mmap_entry*)mmap_addr;
    while ((uint32_t)entry < mmap_addr + mmap_length) {
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            total_mem = entry->addr + entry->len;
        }
        entry = (multiboot_mmap_entry*)((uint32_t)entry + entry->size + sizeof(entry->size));
    }

    pmm_max_blocks = (size_t)(total_mem / PAGE_SIZE);
    pmm_used_blocks = pmm_max_blocks;
    
    // 2. Place bitmap after kernel end
    pmm_bitmap = (uint8_t*)_kernel_end;
    size_t bitmap_size = pmm_max_blocks / 8;

    // Initially mark everything as used (reserved)
    memset(pmm_bitmap, 0xFF, bitmap_size);

    // 3. Parse mmap again to free available regions
    entry = (multiboot_mmap_entry*)mmap_addr;
    while ((uint32_t)entry < mmap_addr + mmap_length) {
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint32_t block_start = (uint32_t)(entry->addr / PAGE_SIZE);
            uint32_t block_count = (uint32_t)(entry->len / PAGE_SIZE);
            
            for (uint32_t i = 0; i < block_count; i++) {
                mmap_unset(block_start + i);
                pmm_used_blocks--;
            }
        }
        entry = (multiboot_mmap_entry*)((uint32_t)entry + entry->size + sizeof(entry->size));
    }

    // 4. Important: Re-lock the kernel and the bitmap itself!
    uint32_t kernel_start_block = (uint32_t)((uint32_t)_kernel_start / PAGE_SIZE);
    // The end of the "reserved" area is kernel end + bitmap size
    uint32_t kernel_end_block = (uint32_t)(((uint32_t)_kernel_end + bitmap_size) / PAGE_SIZE) + 1;

    for (uint32_t i = kernel_start_block; i < kernel_end_block; i++) {
        if (!(pmm_bitmap[i / 8] & (1 << (i % 8)))) {
            mmap_set(i);
            pmm_used_blocks++;
        }
    }

    printf("PMM initialized with %d MB total memory.\n", (uint32_t)(total_mem / 1024 / 1024));
    printf("Bitmap size: %d bytes at 0x%x\n", bitmap_size, (uint32_t)pmm_bitmap);
}

size_t pmm_get_free_memory() {
    return (pmm_max_blocks - pmm_used_blocks) * PAGE_SIZE;
}

// Stubs for next implementation
void* pmm_alloc_page() { return nullptr; }
void pmm_free_page(void* ptr) { (void)ptr; }