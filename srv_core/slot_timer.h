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

#ifndef SLOT_TIMER_H_
#define SLOT_TIMER_H_

#include <assert.h>
#include <stdint.h>

#include "event_handler.h"
#include "accel_req.h"
#include "fd_timer.h"
#include "scheduler.h"
#include "accel_req.h"

//---------------------------------------------------------------------------------------------

struct slot_timer {
    // ------------------------//
    struct event_handler handler;   // Handler interface
    // ------------------------//

    struct fd_timer fd_timer;       // Handle

    // Current executing request
    struct accel_req *exec_req;

    struct scheduler *scheduler;
};

//---------------------------------------------------------------------------------------------

static inline
struct event_handler *slot_timer_get_event_handler(struct slot_timer *self)
{
    assert(self);

    return &self->handler;
}

//---------------------------------------------------------------------------------------------

static inline
int slot_timer_arm(struct slot_timer *self, uint64_t duration_us, struct accel_req *exec_req)
{
    assert(self);
    assert(exec_req);

    self->exec_req = exec_req;

    return fd_timer_arm(&self->fd_timer, duration_us);
}

static inline
int slot_timer_disarm(struct slot_timer *self, uint64_t *elapsed_us)
{
    int retval;

    assert(self);
    assert(elapsed_us);

    self->exec_req = NULL;

    retval = fd_timer_get_elapsed_us(&self->fd_timer, elapsed_us);
    if (retval)
        return -1;

    retval = fd_timer_disarm(&self->fd_timer);
    if (retval)
        return -1;

    return 0;
}

//---------------------------------------------------------------------------------------------

int slot_timer_init(struct slot_timer **self, struct scheduler *scheduler);

//---------------------------------------------------------------------------------------------

#endif /* SLOT_TIMER_H_ */
