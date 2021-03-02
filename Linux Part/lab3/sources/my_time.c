#include "my_time.h"
#include <sys/time.h>

#define NULL (void*)0

double time2double(void) {
    static struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}
