#include "change_lock.h"
#include <malloc.h>
#include <string.h>
#include "spinlock.h"
#include <pthread.h>
#include "my_mutex.h"

int change_lock(char *lock_type, void **lock, lock_func_t *acquire, lock_func_t *release) {
    if (strcmp(lock_type, "pthread_mutex") == 0) {
        free(*lock);
        *lock = malloc(sizeof(pthread_mutex_t));
        *acquire = (lock_func_t)pthread_mutex_lock;
        *release = (lock_func_t)pthread_mutex_unlock;
        pthread_mutex_init(*lock, NULL);
        return 0;
    }
    if (strcmp(lock_type, "pthread_spinlock") == 0) {
        free(*lock);
        *lock = malloc(sizeof(pthread_spinlock_t));
        *acquire = (lock_func_t)pthread_spin_lock;
        *release = (lock_func_t)pthread_spin_unlock;
        pthread_spin_init(*lock, PTHREAD_PROCESS_PRIVATE);
        return 0;
    }
    if (strcmp(lock_type, "spinlock") == 0) {
        free(*lock);
        *lock = malloc(sizeof(spinlock_t));
        *acquire = (lock_func_t)spinlock_acquire;
        *release = (lock_func_t)spinlock_release;
        spinlock_init(*lock);
        return 0;
    }
    if (strcmp(lock_type, "my_mutex") == 0) {
        free(*lock);
        *lock = malloc(sizeof(mutex_t));
        *acquire = (lock_func_t)mutex_acquire;
        *release = (lock_func_t)mutex_release;
        mutex_init(*lock);
        return 0;
    }
    return -1;
}