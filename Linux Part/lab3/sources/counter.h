#ifndef __COUNTER_H__
#define __COUNTER_H__

// function pointer type for general lock acquire & release
typedef void (*lock_func_t)(void*);

typedef struct __counter_t {
    int value;
    // lock used in the counter
    // void* can adapt various types of lock.
    void *lock;
    // lock functions to manipulate the lock.
    // function pointer can adapt various types of lock.
    lock_func_t acquire;
    lock_func_t release;
} counter_t;

void counter_init(counter_t *c, int value);
int counter_get_value(counter_t *c);
void counter_increment(counter_t *c);
void counter_decrement(counter_t *c);

// change the lock of the counter.
// return 0 on success, -1 on failure.
int counter_change_lock(counter_t *c, char *lock_type);

#endif