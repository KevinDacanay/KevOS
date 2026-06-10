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

    // 3. Identity map the first 4MB
    // This includes the kernel image and VGA buffer
    for (uint32_t i = 0; i < 1024; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | VMM_PRESENT | VMM_WRITABLE;
    }

    // 4. Put the Page Table into the Page Directory
    kernel_directory[0] = (uint32_t)first_page_table | VMM_PRESENT | VMM_WRITABLE;

    // 5. Load CR3 with the directory address
    asm volatile("mov %0, %%cr3" : : "r"(kernel_directory));

    // 6. Enable Paging (set PG bit in CR0)
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));

    printf("VMM: Paging enabled. Identity mapped first 4MB.\n");
}

void vmm_map_page(void* phys, void* virt, uint32_t flags) {
    uint32_t pd_index = (uint32_t)virt >> 22;
    uint32_t pt_index = ((uint32_t)virt >> 12) & 0x3FF;

    uint32_t* page_table = nullptr;

    // Check if the page table for this region exists
    if (kernel_directory[pd_index] & VMM_PRESENT) {
        page_table = (uint32_t*)(kernel_directory[pd_index] & ~0xFFF);
    } else {
        // Allocate a new page table if it doesn't exist
        page_table = (uint32_t*)pmm_alloc_page();
        memset(page_table, 0, PAGE_SIZE);
        kernel_directory[pd_index] = (uint32_t)page_table | VMM_PRESENT | VMM_WRITABLE | (flags & VMM_USER);
    }

    // Map the physical address to the virtual address in the page table
    page_table[pt_index] = (uint32_t)phys | (flags & 0xFFF) | VMM_PRESENT;

    // Invalidate the TLB entry for this virtual address
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}