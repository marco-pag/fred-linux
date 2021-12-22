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

#ifndef FD_TIMER_H_
#define FD_TIMER_H_

#include <stdint.h>
#include <assert.h>

//---------------------------------------------------------------------------------------------

struct fd_timer {
    int fd;
    uint64_t duration_us;
};

//---------------------------------------------------------------------------------------------

static inline
int fd_timer_get_fd(struct fd_timer *self)
{
    assert(self);

    return self->fd;
}

//---------------------------------------------------------------------------------------------

int fd_timer_init(struct fd_timer *self);

void fd_timer_free(struct fd_timer *self);

int fd_timer_arm(struct fd_timer *self, uint64_t duration_us);

int fd_timer_disarm(struct fd_timer *self);

int fd_timer_clear_after_timeout(struct fd_timer *self);

int fd_timer_get_elapsed_us(const struct fd_timer *self, uint64_t *elapsed_us);

//---------------------------------------------------------------------------------------------

#endif /* FD_TIMER_H_ */
