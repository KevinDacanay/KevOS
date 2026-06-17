#include <kernel/include/kernel/task.h>
#include <kernel/arch/i386/mm/include/heap.h>
#include <kernel/arch/i386/cpu/include/gdt.h>
#include <stdio.h>

static task_t* current_task = nullptr;
static task_t* ready_queue = nullptr;
static uint32_t next_tid = 1;

extern "C" void tss_set_stack(uint32_t stack);

void tasking_init() {
    // Create the "Initial" task (the current kernel flow)
    current_task = (task_t*)kmalloc(sizeof(task_t));
    current_task->id = next_tid++;
    current_task->running = true;
    current_task->next = current_task; // Circular list
    ready_queue = current_task;
    
    printf("Multitasking initialized. Current Task ID: %d\n", current_task->id);
}

void task_create(void (*entry)()) {
    task_t* new_task = (task_t*)kmalloc(sizeof(task_t));
    new_task->id = next_tid++;
    
    // Allocate 4KB kernel stack
    uint32_t stack_mem = (uint32_t)kmalloc(4096);
    new_task->kstack_bottom = stack_mem;
    
    uint32_t* stack = (uint32_t*)(stack_mem + 4096);

    // Prepare the stack frame for switch_to
    *--stack = (uint32_t)entry; // Return address for ret
    *--stack = 0;               // EBP
    *--stack = 0;               // EBX
    *--stack = 0;               // ESI
    *--stack = 0;               // EDI

    new_task->esp = (uint32_t)stack;
    
    // Add to circular ready queue
    new_task->next = ready_queue->next;
    ready_queue->next = new_task;

    printf("Task created: ID %d, Stack: 0x%x\n", new_task->id, new_task->esp);
}

void schedule() {
    if (!current_task) return;

    task_t* old_task = current_task;
    task_t* new_task = current_task->next;

    if (old_task == new_task) return; // Only one task

    current_task = new_task;
    
    // Update TSS so any interrupts in user mode use this task's stack
    tss_set_stack(new_task->kstack_bottom + 4096);

    // Perform the assembly switch
    switch_to(&old_task->esp, new_task->esp);
}

/**
 * @brief Prints the list of all tasks in the ready queue.
 */
void task_print_list() {
    printf("ID    Stack Pointer    Status\n");
    printf("-----------------------------\n");
    task_t* first = ready_queue;
    task_t* curr = first;
    do {
        printf("%d     0x%x       %s\n", curr->id, curr->esp, curr == current_task ? "Running" : "Ready");
        curr = curr->next;
    } while (curr != first);
}
