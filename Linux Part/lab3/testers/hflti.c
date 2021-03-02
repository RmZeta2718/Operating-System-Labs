// hash test for insert
// each pthread inserts maxn nodes into hash
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "hash_full_lock.h"
#include "my_time.h"

const int maxn = 1e5, max_p = 100, buck_sz = 4096;

hash_t hash;

void *mythread(void *arg) {
    for (int i = 0; i < maxn; ++i)
        hash_insert(&hash, i);
    return NULL;
}

int main(int argc, char *argv[]) {
    // printf("here");
    if (argc != 3) {
        printf("Usage: counter_test [pthread number] [lock name]\n");
        assert(argc == 3);
    }
    // printf("running\n");

    // argv[1] is pthread number.
    char *endptr;
    int cnt_p = strtol(argv[1], &endptr, 10);
    // check if argv[1] is number (see man page of strtol)
    assert(*endptr == '\0');
    assert(0 < cnt_p && cnt_p <= max_p);

    // set up hash
    hash_init(&hash, buck_sz);
    assert(hash_change_lock(&hash, argv[2]) == 0);

    // start hash test.
    double t1 = time2double();
    pthread_t p[max_p];
    // create pthreads.
    for (int i = 0; i < cnt_p; ++i)
        assert(pthread_create(&p[i], NULL, mythread, NULL) == 0);
    // wait pthreads.
    for (int i = 0; i < cnt_p; ++i)
        assert(pthread_join(p[i], NULL) == 0);
    double t2 = time2double();

    // test end, check correctness
    assert(hash_size(&hash) == maxn * cnt_p);

    // clean up
    hash_clear(&hash);

    printf("%d,%.6f\n", cnt_p, t2 - t1);

    return 0;
}