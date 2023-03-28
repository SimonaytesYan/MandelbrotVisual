#ifndef SYM_STOPWATCH
#define SYM_STOPWATCH
#include <sys/time.h>


#define InitTimer()              \
    struct timeval stop, start;
#define StartTimer()             \
    gettimeofday(&start, NULL);

#define StopTimer()              \
    gettimeofday(&stop, NULL);

#define GetTimerMicroseconds()       \
    (((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec))

#endif