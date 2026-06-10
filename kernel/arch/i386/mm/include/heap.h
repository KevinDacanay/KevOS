#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include <stdint.h>
#include <stddef.h>

// Start the heap at 3.25GB in virtual memory
#define KHEAP_START 0xD0000000
#define KHEAP_INITIAL_SIZE (2 * 1024 * 1024) // 2MB

/**
 * @struct heap_node
 * @brief Header for every allocated or free block in the heap.
 */
struct heap_node {
    uint32_t size;      ///< Size of the data block in bytes (excluding header)
    bool free;          ///< Whether the block is available for allocation
    heap_node* next;    ///< Pointer to the next block in the list
    heap_node* prev;    ///< Pointer to the previous block in the list
};

extern "C" {
/**
 * @brief Initializes the kernel heap with an initial pool of memory.
 */
void kheap_init();

/**
 * @brief Allocates a block of memory from the kernel heap.
 */
void* kmalloc(size_t size);

/**
 * @brief Frees a previously allocated block of memory.
 */
void kfree(void* ptr);
}

#endif