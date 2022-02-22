#pragma once

#define LOCKED      1
#define UNLOCKED    0

typedef struct __lock {
    int flag;
} Lock;

#define NewLock         (Lock){UNLOCKED}
#define LockOperation(l, operation)         lock(&l);           \
                                            operation;          \
                                            unlock(&l);

void lock_init(Lock *lock);
void lock(Lock* lock);
void unlock(Lock* lock);
