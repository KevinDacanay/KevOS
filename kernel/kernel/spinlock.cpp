#include <kernel/include/kernel/spinlock.h>

void spinlock_acquire(spinlock_t* lock) {
    // __sync_lock_test_and_set is a GCC atomic builtin.
    // It sets the lock to 1 and returns the previous value.
    while (__sync_lock_test_and_set(lock, 1)) {
        // Spin until the lock is released (value becomes 0)
    }
}

void spinlock_release(spinlock_t* lock) {
    __sync_lock_release(lock);
}