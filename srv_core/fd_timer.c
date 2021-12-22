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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/timerfd.h>

#include "fd_timer.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

int fd_timer_arm(struct fd_timer *self, uint64_t duration_us)
{
    int retval;
    struct itimerspec timer_spec;

    assert(self);

    self->duration_us = duration_us;

    timer_spec.it_value.tv_sec = duration_us / 1000000;
    timer_spec.it_value.tv_nsec = (duration_us % 1000000) * 1000;

    timer_spec.it_interval.tv_sec = 0;
    timer_spec.it_interval.tv_nsec = 0;

    retval = timerfd_settime(self->fd, 0, &timer_spec, NULL);
    if (retval) {
        ERROR_PRINT("fred_sys: unable to arm timerfd. Error: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int fd_timer_disarm(struct fd_timer *self)
{
    int retval;
    struct itimerspec timer_spec;

    assert(self);

    // Setting both fields of new_value.it_value to zero disarms the timer
    timer_spec.it_value.tv_sec = 0;
    timer_spec.it_value.tv_nsec = 0;

    timer_spec.it_interval.tv_sec = 0;
    timer_spec.it_interval.tv_nsec = 0;

    retval = timerfd_settime(self->fd, 0, &timer_spec, NULL);
    if (retval) {
        ERROR_PRINT("fred_sys: unable to disarm timerfd. Error: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int fd_timer_clear_after_timeout(struct fd_timer *self)
{
    int retval;
    uint64_t num_expired;

    assert(self);

    // the buffer given to read returns an unsigned 8-byte integer
    // (uint64_t) containing the number of expirations that have occurred
    retval = read(self->fd, &num_expired, sizeof(num_expired));
    if (retval != sizeof(num_expired)) {
        ERROR_PRINT("fred_sys: unable to read timerfd. Error: %s\n", strerror(errno));
        return -1;
    }

    // By construction, fd_timer can expire only one time
    assert(num_expired == 1U);

    return 0;
}

int fd_timer_get_elapsed_us(const struct fd_timer *self, uint64_t *elapsed_us)
{
    int retval;
    uint64_t remaining_us;
    struct itimerspec timer_spec;

    assert(self);

    retval = timerfd_gettime(self->fd, &timer_spec);
    if (retval) {
        ERROR_PRINT("fred_sys: unable get timerfd elapsed time."
                    " Error: %s\n", strerror(errno));
        *elapsed_us = 0;
        return -1;
    }

    remaining_us = timer_spec.it_value.tv_sec * 1000000 + timer_spec.it_value.tv_nsec / 1000;
    *elapsed_us = self->duration_us - remaining_us;

    return 0;
}

int fd_timer_init(struct fd_timer *self)
{
    self->fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (self->fd < 0) {
        ERROR_PRINT("fred_sys: unable to create timerfd. Error: %s\n", strerror(errno));
        free(self);
        return -1;
    }

    return 0;
}

void fd_timer_free(struct fd_timer *self)
{
    if (self)
        close(self->fd);
}
