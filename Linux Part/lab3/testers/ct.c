// counter test
// each pthread does increment maxn times
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "counter.h"
#include "my_time.h"

counter_t counter;
const int maxn = 1e6, max_p = 100;

void *mythread(void *arg) {
    for (int i = 0; i < maxn; ++i)
        counter_increment(&counter);
    return NULL;
}

int main(int argc, char *argv[]) {
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

    // set up counter
    counter_init(&counter, 0);
    assert(counter_change_lock(&counter, argv[2]) == 0);

    // start counter test.
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
    assert(counter_get_value(&counter) == maxn * cnt_p);

    printf("%d,%.6f\n", cnt_p, t2 - t1);

    return 0;
}