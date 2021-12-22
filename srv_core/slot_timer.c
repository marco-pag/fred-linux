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

#include <stdlib.h>
#include <stdio.h>

#include "slot_timer.h"

// ---------------------- Functions to implement event_handler interface ----------------------

static
int get_fd_handle_(const struct event_handler *self)
{
    struct slot_timer *timer;

    assert(self);

    timer = (struct slot_timer *)self;
    return fd_timer_get_fd(&timer->fd_timer);
}

static
int handle_event_(struct event_handler *self)
{
    struct slot_timer *timer;
    int retval;

    assert(self);

    timer = (struct slot_timer *)self;

    // Consume the event from the fd
    retval = fd_timer_clear_after_timeout(&timer->fd_timer);
    if (retval)
        return -1;

    // Signal the scheduler
    retval = scheduler_slot_timeout(timer->scheduler, timer->exec_req);

    return retval;
}

static
void get_name_(const struct event_handler *self, char *msg, int msg_size)
{
    struct slot_timer *timer;

    assert(self);

    timer = (struct slot_timer *)self;
    snprintf(msg, msg_size, "slot timer on fd: %d", fd_timer_get_fd(&timer->fd_timer));
}

static
void free_(struct event_handler *self)
{
    struct slot_timer *timer;

    if (!self)
        return;

    timer = (struct slot_timer *)self;

    fd_timer_free(&timer->fd_timer);
    free(timer);
}

//---------------------------------------------------------------------------------------------

int slot_timer_init(struct slot_timer **self, struct scheduler *scheduler)
{
    int retval;

    assert(scheduler);

    // Allocate and set everything to 0
    *self = calloc(1, sizeof (**self));
    if (!(*self))
        return -1;

    retval = fd_timer_init(&(*self)->fd_timer);
    if (retval) {
        free(*self);
        return -1;
    }

    (*self)->scheduler = scheduler;

    // Event handler interface
    event_handler_assign_id(&(*self)->handler);
    (*self)->handler.handle_event = handle_event_;
    (*self)->handler.get_fd_handle = get_fd_handle_;
    (*self)->handler.get_name = get_name_;
    (*self)->handler.free = free_;

    return 0;
}
