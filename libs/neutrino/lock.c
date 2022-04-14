#include "lock.h"
#include <neutrino/macros.h>

// === PRIVATE FUNCTIONS ========================

int unoptimized test_and_set(int* old_flag, int new_v) {
    int old = *old_flag;
    *old_flag = new_v;
    return old;
}

// === PUBLIC FUNCTIONS =========================

//* Initialize the lock to UNLOCKED
// @param lock the lock to be initialized 
void lock_init(Lock *lock) {
    lock->flag = UNLOCKED;
}

// *Lock a spinlock, avoiding other threads accessing it
// @param lock the lock to be locked
void unoptimized lock(Lock* lock) {
    while (!__sync_bool_compare_and_swap(&(lock->flag), UNLOCKED, LOCKED))
        asm volatile ("pause");
}

// *Unlock a spinlock and make it available to other threads
// @param lock the lock to be unlocked
void unoptimized unlock(Lock* lock) {
    lock->flag = UNLOCKED;
}
