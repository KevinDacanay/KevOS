#ifndef KERNEL_SPINLOCK_H
#define KERNEL_SPINLOCK_H

#include <stdint.h>

/**
 * @brief Simple spinlock structure.
 * 0 = unlocked, 1 = locked.
 */
typedef volatile int spinlock_t;

/**
 * @brief Acquires the spinlock. 
 * Busy-waits until the lock is acquired.
 */
void spinlock_acquire(spinlock_t* lock);

/**
 * @brief Releases the spinlock.
 */
void spinlock_release(spinlock_t* lock);

#endif