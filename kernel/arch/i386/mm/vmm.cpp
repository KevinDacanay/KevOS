/**
 * @file vmm.cpp
 * @brief Virtual Memory Manager (Paging) implementation.
 * 
 * This file manages page directories and page tables. It uses a 
 * recursive mapping trick (slot 1023) to allow the kernel to access 
 * and modify page tables using virtual addresses.
 */
#include <kernel/arch/i386/mm/include/vmm.h>
#include <kernel/arch/i386/mm/include/pmm.h>
#include <string.h>
#include <stdio.h>

static uint32_t* kernel_directory = nullptr;

void vmm_init() {
    // 1. Allocate a page for the Page Directory
    kernel_directory = (uint32_t*)pmm_alloc_page();
    memset(kernel_directory, 0, PAGE_SIZE);

    // 2. Allocate a page for the first Page Table (covers first 4MB)
    uint32_t* first_page_table = (uint32_t*)pmm_alloc_page();
    memset(first_page_table, 0, PAGE_SIZE);

    // 3. Identity map the first 4MB of physical memory.
    // This is required so the CPU can transition from physical to virtual addresses smoothly.
    for (uint32_t i = 0; i < 1024; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | VMM_PRESENT | VMM_WRITABLE;
    }

    // 4. Set up the Page Directory entries
    kernel_directory[0] = (uint32_t)first_page_table | VMM_PRESENT | VMM_WRITABLE;
    // Higher-half mapping: Map 3GB virtual to 0GB physical
    kernel_directory[KERNEL_VIRT_BASE >> 22] = (uint32_t)first_page_table | VMM_PRESENT | VMM_WRITABLE;

    // Recursive mapping: The 1023rd entry points back to the directory.
    // This makes PD accessible at 0xFFFFF000 and PTs at 0xFFC00000.
    kernel_directory[VMM_RECURSIVE_SLOT] = (uint32_t)kernel_directory | VMM_PRESENT | VMM_WRITABLE;

    // 5. Load CR3 with the directory address
    asm volatile("mov %0, %%cr3" : : "r"(kernel_directory));

    // 6. Enable Paging (set PG bit in CR0)
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));

    // Transition global pointer to the recursive virtual address so it remains valid.
    kernel_directory = (uint32_t*)VMM_PAGE_DIRECTORY;

    printf("VMM: Paging enabled. Higher-half kernel mapped at 0x%x\n", KERNEL_VIRT_BASE);
}

/**
 * @brief Maps a physical page to a virtual address.
 * 
 * Uses the recursive mapping to check for the existence of page tables.
 * If a table is missing, it is allocated and mapped on the fly.
 * 
 * @param phys The physical frame address.
 * @param virt The virtual address to map to.
 * @param flags Permission flags (VMM_PRESENT, VMM_WRITABLE, etc).
 */
void vmm_map_page(void* phys, void* virt, uint32_t flags) {
    uint32_t pd_index = (uint32_t)virt >> 22;
    uint32_t pt_index = ((uint32_t)virt >> 12) & 0x3FF;

    uint32_t* pd = (uint32_t*)VMM_PAGE_DIRECTORY;
    uint32_t* pts = (uint32_t*)VMM_PAGE_TABLES;

    // Check if the page table for this region exists
    if (!(pd[pd_index] & VMM_PRESENT)) {
        // Allocate a new page table if it doesn't exist
        uint32_t pt_phys = (uint32_t)pmm_alloc_page();
        pd[pd_index] = pt_phys | VMM_PRESENT | VMM_WRITABLE | (flags & VMM_USER);
        
        // Using recursive mapping, the new page table is now accessible at a virtual address
        uint32_t* new_pt_virt = pts + (pd_index * 1024);
        memset(new_pt_virt, 0, PAGE_SIZE);
    }

    // Access the specific page table through the recursive PT area
    pts[pd_index * 1024 + pt_index] = (uint32_t)phys | (flags & 0xFFF) | VMM_PRESENT;

    // Invalidate TLB for the changed mapping
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}