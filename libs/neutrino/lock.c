#include "lock.h"

// === PRIVATE FUNCTIONS ========================

int test_and_set(int* old_flag, int new_v) {
    int old = *old_flag;
    *old_flag = new_v;
    return old;
}

// === PUBLIC FUNCTIONS =========================

//* Initialize the lock to UNLOCKED
// @param lock the lock to be initialized 
void lock_init(lock_t *lock) {
    lock->flag = UNLOCKED;
}

// *Lock a spinlock, avoiding other threads accessing it
// @param lock the lock to be locked
void lock(lock_t* lock) {
    while (test_and_set(&lock->flag, LOCKED) == 1);
}

// *Unlock a spinlock and make it available to other threads
// @param lock the lock to be unlocked
void unlock(lock_t* lock) {
    lock->flag = UNLOCKED;
}
