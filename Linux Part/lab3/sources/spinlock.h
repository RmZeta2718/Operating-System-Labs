#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

typedef struct __spinlock_t {
    unsigned int flag;
} spinlock_t;

void spinlock_init(spinlock_t *lock);
void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);

#endif