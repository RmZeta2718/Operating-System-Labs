#include "my_mutex.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <time.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

// basic wrapper for futex
static long
futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3) {
    return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}

static inline uint
xchg(volatile unsigned int *addr, unsigned int newval)
{
    uint result;
    asm volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
    return result;
}

#define LOCK_FREE 0
#define LOCK_HELD 1

void mutex_init(mutex_t *lock) {
    lock->number = LOCK_FREE;
}

void mutex_acquire(mutex_t *lock) {
    long s;
    while (1) {
        /* Is the futex available? */
        if (xchg(&lock->number, LOCK_HELD) == LOCK_FREE)
            return;      /* Yes */

        /* Futex is not available; wait */
        s = futex(&lock->number, FUTEX_WAIT, LOCK_HELD, NULL, NULL, 0);
        if (s == -1 && errno != EAGAIN)
            errExit("futex-FUTEX_WAIT");
    }
}

void mutex_release(mutex_t *lock) {
    long s;
    if (xchg(&lock->number, LOCK_FREE) == LOCK_HELD) {
        s = futex(&lock->number, FUTEX_WAKE, 1, NULL, NULL, 0);
        if (s  == -1)
            errExit("futex-FUTEX_WAKE");
    }
}

// high level wrappers for sys_futex()
// static long futex_wait(void *addr, int val) {
//     return sys_futex(addr, FUTEX_WAIT, val, NULL, NULL, 0);
// }
// static long futex_wake(void *addr) {
//     return sys_futex(addr, FUTEX_WAKE, 1, NULL, NULL, 0);
// }

// void mutex_acquire(mutex_t *lock) {
//     int v;
//     /* Bit 31 was clear, we got the mutex (the fastpath) */
//     // spinlock: spin for one round
//     if (atomic_bit_test_set(&lock->number, 31) == 0)
//         return;
//     // number is the length of wait queue
//     atomic_increment(&lock->number);
//     while (1) {
//         // wake up by release()
//         // acquire lock and maintain number (wait queue length)
//         if (atomic_bit_test_set(&lock->number, 31) == 0) {
//             atomic_decrement(&lock->number);
//             return;
//         }

//         /* We have to waitFirst make sure the futex value
//         we are monitoring is truly negative (locked). */

//         // others might call release() here (?)
//         v = lock->number;
//         if (v >= 0)
//             continue;

//         futex_wait(&lock->number, v);
//     }

// }

// void mutex_release(mutex_t *lock) {
//     /* Adding 0x80000000 to counter results in 0 if and
//     only if there are not other interested threads */
//     if (atomic_add_zero(&lock->number, 0x80000000))
//         return;

//     /* There are other threads waiting for this mutex,
//     wake one of them up. */
//     futex_wake(&lock->number);
// }