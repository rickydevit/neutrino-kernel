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
    while (atomic_test_and_set((volatile uint8_t*)&(lock->flag))) {
        asm volatile ("pause");
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
    }
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
}

// *Unlock a spinlock and make it available to other threads
// @param lock the lock to be unlocked
void unoptimized unlock(Lock* lock) {
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    atomic_release((volatile uint8_t*)&(lock->flag));
}

// *Return true if the lock is UNLOCKED
// @param lock the lock to be checked
// @return true if the lock is UNLOCKED, false otherwise
bool unoptimized try_lock(Lock *lock) {
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    return (atomic_get_byte((volatile uintptr_t)(volatile uint8_t*)&(lock->flag)) == UNLOCKED);
}
