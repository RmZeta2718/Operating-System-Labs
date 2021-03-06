// list test for delete
// first insert maxn*cnt_p nodes into list
// then, each pthread deletes those nodes in the same order
// ie. in the reverse order to the list
// insertion time is not counted
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "list.h"
#include "my_time.h"

list_t list;
const int maxn = 1e3, max_p = 100;

void *mythread(void *arg) {
    for (int i = 0; i < maxn; ++i)
        list_delete(&list, i);
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

    // set up list
    list_init(&list);
    assert(list_change_lock(&list, argv[2]) == 0);
    // insert nodes for delete
    for (int i = 0; i < maxn; ++i)
        for (int t = 0; t < cnt_p; ++t)
            list_insert(&list, i);
    assert(list_size(&list) == maxn * cnt_p);

    // start list test.
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
    assert(list_size(&list) == 0);

    printf("%d,%.6f\n", cnt_p, t2 - t1);

    return 0;
}