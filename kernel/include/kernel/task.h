#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Represents the state of a task (TCB).
 */
struct task_t {
    uint32_t esp;            ///< Current stack pointer for the task
    uint32_t kstack_bottom;  ///< Base address of the kernel stack
    uint32_t id;             ///< Unique task ID
    struct task_t* next;     ///< Link to the next task in the ready queue
    bool running;            ///< Execution state
};

/**
 * @brief Initializes the multitasking system.
 */
void tasking_init();

/**
 * @brief Creates a new kernel task.
 * @param entry The function where the task begins execution.
 */
void task_create(void (*entry)());

/**
 * @brief Voluntarily yields the CPU to the next task.
 */
void schedule();

/**
 * @brief Prints the list of all tasks in the ready queue.
 */
void task_print_list();

extern "C" {
    void switch_to(uint32_t* old_esp, uint32_t new_esp);
}

#endif
