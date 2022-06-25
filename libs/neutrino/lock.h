#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <neutrino/macros.h>
#include <_null.h>

#define LOCKED      1
#define UNLOCKED    0

typedef struct __lock {
    volatile uint8_t flag;
} Lock;

#define NewLock         (Lock){UNLOCKED}

void lock_init(Lock *lock);
void lock(Lock* lock);
void unlock(Lock* lock);
bool try_lock(Lock *lock);

static inline void retainer_release(Lock** l) {
    if (l != nullptr) {
        unlock(*l);
        *l = nullptr;
    }
}

#define _LockRetain(ret, l)    \
    Lock* ret cleanup(retainer_release) = l;    \
    lock(ret);

#define LockRetain(l)    \
    _LockRetain(Concat(retainer, __COUNTER__), &l)
    
#define LockOperation(l, operation)         lock(&l);           \
                                            operation;          \
                                            unlock(&l);
