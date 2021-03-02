#include <stdlib.h>
#include "spinlock.h"

static inline uint
xchg(volatile unsigned int *addr, unsigned int newval)
{
    uint result;
    asm volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
    return result;
}

#define LOCK_FREE 0
#define LOCK_HELD 1

void spinlock_init(spinlock_t *lock) {
    lock->flag = LOCK_FREE;
}

void spinlock_acquire(spinlock_t *lock) {
    while (xchg(&lock->flag, LOCK_HELD) == LOCK_HELD);
}

void spinlock_release(spinlock_t *lock) {
    lock->flag = LOCK_FREE;
}