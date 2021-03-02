#ifndef __MY_MUTEX_H__
#define __MY_MUTEX_H__

typedef struct __mutex_t {
    unsigned int number;
} mutex_t;

void mutex_init(mutex_t *lock);
void mutex_acquire(mutex_t *lock);
void mutex_release(mutex_t *lock);

#endif