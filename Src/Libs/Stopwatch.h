#ifndef SYM_STOPWATCH
#define SYM_STOPWATCH
#include <sys/time.h>

//! chrono
//! sfml timer

#define InitTimer()              \
    clock_t startTime, endTime;

#define StartTimer()             \
        startTime = clock();

#define StopTimer()              \
        endTime   = clock();

#define GetTimerMicroseconds()       \
    (((double)endTime - (double)startTime) / CLOCKS_PER_SEC * 1000)

#endif