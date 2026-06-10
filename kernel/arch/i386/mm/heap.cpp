/**
 * @file heap.cpp
 * @brief Kernel Heap Allocator implementation.
 * 
 * This module implements a doubly-linked list based block allocator for the kernel.
 * Features:
 * - First-fit allocation strategy.
 * - Block splitting to minimize internal fragmentation.
 * - Immediate coalescing (merging) of adjacent free blocks on kfree().
 * - Dynamic expansion via VMM when the heap is exhausted.
 */
#include <kernel/arch/i386/mm/include/heap.h>
#include <kernel/arch/i386/mm/include/vmm.h>
#include <kernel/arch/i386/mm/include/pmm.h>
#include <string.h>
#include <stdio.h>

static heap_node* heap_head = nullptr;
static uint32_t heap_current_end = KHEAP_START; ///< Tracks the current top of the mapped heap area.

/**
 * @brief Expands the heap by a specific number of pages.
 * 
 * Requests physical frames from the PMM and maps them into the virtual 
 * heap address space. This increases the total addressable memory of the heap.
 * 
 * @param pages Number of 4KB pages to add to the heap.
 */
static void kheap_expand(size_t pages) {
    for (size_t i = 0; i < pages; i++) {
        void* phys = pmm_alloc_page();
        if (!phys) {
            printf("KERNEL PANIC: Out of physical memory during heap expansion!\n");
            for (;;);
        }
        vmm_map_page(phys, (void*)heap_current_end, VMM_PRESENT | VMM_WRITABLE);
        heap_current_end += PAGE_SIZE;
    }
}

/**
 * @brief Initializes the kernel heap.
 * 
 * Sets up the initial memory pool and creates the first "mega-block" 
 * representing the entire free range.
 */
void kheap_init() {
    // Map initial heap pages
    size_t initial_pages = KHEAP_INITIAL_SIZE / PAGE_SIZE;
    kheap_expand(initial_pages);

    // Create the initial large free block
    heap_head = (heap_node*)KHEAP_START;
    heap_head->size = KHEAP_INITIAL_SIZE - sizeof(heap_node);
    heap_head->free = true;
    heap_head->next = nullptr;
    heap_head->prev = nullptr;

    printf("Kernel Heap initialized: %d MB at 0x%x\n", KHEAP_INITIAL_SIZE / 1024 / 1024, KHEAP_START);
}

/**
 * @brief Allocates memory of the requested size.
 * 
 * Uses a first-fit search. If a suitable block is found, it may be split 
 * if the remaining space is large enough to hold another metadata header.
 * If no block is found, the heap is expanded.
 * 
 * @param size Minimum bytes required.
 * @return Pointer to the allocated data, or nullptr.
 */
void* kmalloc(size_t size) {
    if (size == 0) return nullptr;

    size = (size + 7) & ~7;

    heap_node* curr = heap_head;
    while (curr) {
        if (curr->free && curr->size >= size) {
            // Splitting logic: Only split if the remainder is large enough for a header + data
            if (curr->size >= size + sizeof(heap_node) + 8) {
                heap_node* new_node = (heap_node*)((uint32_t)curr + sizeof(heap_node) + size);
                new_node->size = curr->size - size - sizeof(heap_node);
                new_node->free = true;
                new_node->next = curr->next;
                new_node->prev = curr;

                if (curr->next) curr->next->prev = new_node;
                curr->next = new_node;
                curr->size = size;
            }

            curr->free = false;
            return (void*)((uint32_t)curr + sizeof(heap_node));
        }
        
        // If we reach the end and found nothing, expand the heap
        if (!curr->next) {
            size_t pages_needed = (size + sizeof(heap_node) + PAGE_SIZE - 1) / PAGE_SIZE;
            uint32_t old_end = heap_current_end;
            kheap_expand(pages_needed);

            // Create a new node in the expanded area
            heap_node* extension = (heap_node*)old_end;
            extension->size = (pages_needed * PAGE_SIZE) - sizeof(heap_node);
            extension->free = true;
            extension->next = nullptr;
            extension->prev = curr;
            curr->next = extension;

            // Re-loop to allocate from the new space (or coalesce)
            // For simplicity, we trigger kfree on it to trigger coalescing
            kfree((void*)((uint32_t)extension + sizeof(heap_node)));
            curr = heap_head; // Restart search
            continue;
        }
        curr = curr->next;
    }

    return nullptr;
}

/**
 * @brief Returns memory to the heap.
 * 
 * Marks the block as free and immediately checks adjacent blocks 
 * (next and previous) to merge them into a single larger block.
 * 
 * @param ptr Pointer previously returned by kmalloc.
 */
void kfree(void* ptr) {
    if (!ptr) return;

    heap_node* node = (heap_node*)((uint32_t)ptr - sizeof(heap_node));
    node->free = true;

    if (node->next && node->next->free) {
        node->size += node->next->size + sizeof(heap_node);
        node->next = node->next->next;
        if (node->next) node->next->prev = node;
    }

    if (node->prev && node->prev->free) {
        node->prev->size += node->size + sizeof(heap_node);
        node->prev->next = node->next;
        if (node->next) node->next->prev = node->prev;
    }
}