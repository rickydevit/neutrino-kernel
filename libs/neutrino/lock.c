#include "lock.h"
#include <neutrino/macros.h>
#include <neutrino/atomic.h>

// === PRIVATE FUNCTIONS ========================

// === PUBLIC FUNCTIONS =========================

//* Initialize the lock to UNLOCKED
// @param lock the lock to be initialized 
void lock_init(Lock *lock) {
    lock->flag = UNLOCKED;
}

// *Lock a spinlock, avoiding other threads accessing it
// @param lock the lock to be locked
void unoptimized lock(Lock* lock) {
    while (atomic_test_and_set(&(lock->flag)))
        asm volatile ("pause");
}

// *Unlock a spinlock and make it available to other threads
// @param lock the lock to be unlocked
void unoptimized unlock(Lock* lock) {
    atomic_release(&(lock->flag));
}
