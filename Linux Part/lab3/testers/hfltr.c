// hash test for random insert and delete
// random numbers are generated in advance
// each thread inserts its own random numbers
// and randomly delete them, which order is predefined in advance
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "hash_full_lock.h"
#include "my_time.h"

const int maxn = 1e4, max_p = 100, buck_sz = 100;
const unsigned seed = 352481532U;

hash_t hash;
int **insert, **delete;

void *mythread(void *arg) {
    int idx_p = *(int*)arg;
    for (int i = 0; i < maxn; ++i)
        hash_insert(&hash, insert[idx_p][i]);
    for (int i = 0; i < maxn; ++i)
        hash_delete(&hash, delete[idx_p][i]);
    return NULL;
}

void shuffle(int *array, int n);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: counter_test [pthread number] [lock name]\n");
        assert(argc == 3);
    }
    // printf("running\n");

    // argv[1] is pthread number
    char *endptr;
    int cnt_p = strtol(argv[1], &endptr, 10);
    // check if argv[1] is number (see man page of strtol)
    assert(*endptr == '\0');
    assert(0 < cnt_p && cnt_p <= max_p);

    // set up hash
    hash_init(&hash, buck_sz);
    assert(hash_change_lock(&hash, argv[2]) == 0);

    // generate random numbers
    // avoid run time rand() and missing deletes
    // Is rand() thread safe?
    srand(seed);
    insert = malloc(sizeof(int*) * cnt_p);
    delete = malloc(sizeof(int*) * cnt_p);
    for (int i = 0; i < cnt_p; ++i) {
        insert[i] = malloc(sizeof(int) * maxn);
        delete[i] = malloc(sizeof(int) * maxn);
        for (int j = 0; j < maxn; ++j)
            insert[i][j] = delete[i][j] = rand();
        shuffle(delete[i], maxn);
    }
    // for (int i = 0; i < cnt_p; ++i) for (int j = 0; j < maxn; ++j)
    //     printf("%d%c", insert[i][j], " \n"[j == maxn - 1]);
    // for (int i = 0; i < cnt_p; ++i) for (int j = 0; j < maxn; ++j)
    //     printf("%d%c", delete[i][j], " \n"[j == maxn - 1]);
    // return 0;

    // generate index array for pthread parameter
    int idx[max_p];
    for (int i = 0; i < cnt_p; ++i)
        idx[i] = i;

    // start hash test.
    double t1 = time2double();
    pthread_t p[max_p];
    // create pthreads.
    for (int i = 0; i < cnt_p; ++i)
        assert(pthread_create(&p[i], NULL, mythread, &idx[i]) == 0);
    // wait pthreads.
    for (int i = 0; i < cnt_p; ++i)
        assert(pthread_join(p[i], NULL) == 0);
    double t2 = time2double();

    // test end, check correctness
    assert(hash_size(&hash) == 0);

    // clean up
    for (int i = 0; i < cnt_p; ++i) {
        free(insert[i]);
        free(delete[i]);
    }
    free(insert);
    free(delete);

    printf("%d,%.6f\n", cnt_p, t2 - t1);

    return 0;
}

// random shuffle an array, pasted from the following link
// https://benpfaff.org/writings/clc/shuffle.html
void shuffle(int *array, int n) {
    for (int i = 0; i < n - 1; i++) {
        int j = i + rand() / (RAND_MAX / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}