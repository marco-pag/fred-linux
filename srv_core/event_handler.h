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

#ifndef EVENT_HANDLER_H_
#define EVENT_HANDLER_H_

#include <assert.h>

//---------------------------------------------------------------------------------------------
// Event handler interface

// handle_event() must return:
// 0 if event has been proper handled
// < 0 in case of system error -> reactor event loop shutdown
// > 0 in case of single handler error (or remove request) -> reactor must detach handler

// In case of error or disconnection request the reactor relies on the
// free() method to (eventually) deallocate the event handler after deregistering

//---------------------------------------------------------------------------------------------

struct event_handler {
    int id;

    int (*get_fd_handle)(const struct event_handler *self);
    int (*handle_event)(struct event_handler *self);

    void (*get_name)(const struct event_handler *self, char *msg, int msg_size);
    void (*free)(struct event_handler *self);
};

//---------------------------------------------------------------------------------------------

static inline
void event_handler_assign_id(struct event_handler *self)
{
    static int id_counter = 0;

    assert(self);

    self->id = id_counter++;
}

static inline
int event_handler_get_id(const struct event_handler *self)
{
    assert(self);

    return self->id;
}

static inline
int event_handler_get_fd_handle(const struct event_handler *self)
{
    assert(self);

    return self->get_fd_handle(self);
}

static inline
int event_handler_handle_event(struct event_handler *self)
{
    assert(self);

    return self->handle_event(self);
}

static inline
void event_handler_get_name(const struct event_handler *self, char *msg, int msg_size)
{
    assert(self);
    assert(msg);

    self->get_name(self, msg, msg_size);
}

static inline
void event_handler_free(struct event_handler *self)
{
    assert(self);

    return self->free(self);
}

//---------------------------------------------------------------------------------------------

#endif /* EVENT_HANDLER_H_ */
