#include "counter.h"
#include "change_lock.h"
#include "my_time.h"

#define NULL (void*)(0)

void counter_init(counter_t *c, int value) {
    c->value = value;
    c->lock = NULL;
    counter_change_lock(c, "pthread_mutex");
}

int counter_get_value(counter_t *c) {
    int value;
    c->acquire(c->lock);
    value = c->value;
    c->release(c->lock);
    return value;
}

void counter_increment(counter_t *c) {
    c->acquire(c->lock);
    ++c->value;
    c->release(c->lock);
}

void counter_decrement(counter_t *c) {
    c->acquire(c->lock);
    --c->value;
    c->release(c->lock);
}

int counter_change_lock(counter_t *c, char *lock_type) {
    return change_lock(lock_type, &c->lock, &c->acquire, &c->release);
}