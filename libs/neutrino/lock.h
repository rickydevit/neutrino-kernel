#pragma once

#define LOCKED      1
#define UNLOCKED    0

typedef struct __lock {
    int flag;
} Lock;

#define NewLock (Lock){UNLOCKED}

void lock_init(Lock *lock);
void lock(Lock* lock);
void unlock(Lock* lock);
