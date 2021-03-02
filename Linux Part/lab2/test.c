#include <stdio.h>
#include "mem.h"

// a fancy way to debug
#define DEBUG
#ifdef DEBUG
#define here printf("Passing [%s] in Line %d\n", __FUNCTION__, __LINE__);
#else
#define here ;
#endif

int main() {
    int size_of_region = 4096;
    mem_init(size_of_region);
    mem_dump();
    void* p[100];
    for (int i = 0; i < 10; ++i) {
        p[i] = mem_alloc(16, i % 3);
        printf("p[%d]: %p\n", i, p[i]);
    }
    here
    mem_dump();
    for (int i = 9; i >= 0; --i) {
        mem_free(p[i]);
    }
    mem_dump();
    
    for (int i = 0; i < 10; ++i) {
        p[i] = mem_alloc(100, i % 3);
        printf("p[%d]: %p\n", i, p[i]);
    }
    here
    mem_dump();
    mem_free(p[1]);
    mem_free(p[7]);
    mem_free(p[8]);
    mem_dump();
    printf("%p\n", mem_alloc(16, M_WORSTFIT));
    printf("%p\n", mem_alloc(16, M_BESTFIT));
    printf("%p\n", mem_alloc(16, M_FIRSTFIT));
    printf("%p\n", mem_alloc(200, M_FIRSTFIT));
    mem_dump();
    mem_free(p[3]);
    mem_free(p[2]);
    mem_dump();
    return 0;
}