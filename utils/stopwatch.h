/*
 * Fred for Linux. Experimental support.
 *
 * Copyright (C) 2018-2021, Marco Pagani, ReTiS Lab.
 * <marco.pag(at)outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
*/

#ifndef STOPWATCH_H_
#define STOPWATCH_H_

//#define __USE_POSIX199309
#include <time.h>

typedef struct stopwatch_ stopwatch;

struct stopwatch_ {
    struct timespec t_start;
    struct timespec t_stop;
};

static inline
void stopwatch_start(stopwatch *watch)
{
    clock_gettime(CLOCK_MONOTONIC, &watch->t_start);
}

static inline
void stopwatch_stop(stopwatch *watch)
{
    clock_gettime(CLOCK_MONOTONIC, &watch->t_stop);
}

static inline
uint64_t stopwatch_get_ms(stopwatch *watch)
{
    return  ((watch->t_stop.tv_sec - watch->t_start.tv_sec) * 1000) +
            ((watch->t_stop.tv_nsec - watch->t_start.tv_nsec) / 1000000);
}

static inline
uint64_t stopwatch_get_us(stopwatch *watch)
{
    return  ((watch->t_stop.tv_sec - watch->t_start.tv_sec) * 1000) +
            ((watch->t_stop.tv_nsec - watch->t_start.tv_nsec) / 1000);
}


#endif /* STOPWATCH_H_ */
