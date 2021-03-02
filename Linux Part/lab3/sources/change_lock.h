#ifndef __CHANGE_LOCK_H__
#define __CHANGE_LOCK_H__

// function pointer type for general lock acquire & release
typedef void (*lock_func_t)(void*);

// Check lock type. If match, malloc mem for the new lock, 
// set lock functions, and finally init the lock.
int change_lock(char *lock_type, void **lock, lock_func_t *acquire, lock_func_t *release);

#endif