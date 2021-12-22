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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <poll.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "reactor.h"
#include "event_handler.h"
#include "../parameters.h"
#include "../utils/dbg_print.h"

#ifdef USE_POLL

//---------------------------------------------------------------------------------------------

// Poll implementation of the reactor

//---------------------------------------------------------------------------------------------

// Internal event handler wrapper
struct event_source_ {
    int active;
    struct event_handler *handler;
};

//---------------------------------------------------------------------------------------------

struct reactor {
    // Active handlers count
    int events_count;

    // Events handlers repository
    struct event_source_ events_sources[MAX_EVENTS_SRCS];

    // pollfd array for poll
    struct pollfd events_fds[MAX_EVENTS_SRCS];
};

//--- Private methods -------------------------------------------------------------------------

// This is supposed to happens only in case of *faults* when a client crashes
static
void free_event_source_(struct reactor *self, int i)
{
    char handler_name[MAX_NAMES];
    struct event_source_ *event_source;

    // Get event source from the repository
    event_source = &self->events_sources[i];

    // Get handler name for print
    event_source->handler->get_name(event_source->handler, handler_name, MAX_NAMES);

    DBG_PRINT("fred_sys: poll reactor: removing event handler %s\n", handler_name);

    // Free the event handler object
    event_source->handler->free(event_source->handler);

    // Remove event source and compact arrays
    memmove(&self->events_sources[i],
            &self->events_sources[i + 1],
            (self->events_count - 1 - i) * sizeof(self->events_sources[0]));

    memmove(&self->events_fds[i],
            &self->events_fds[i + 1],
            (self->events_count - 1 - i) * sizeof(self->events_fds[0]));

    self->events_sources[self->events_count - 1].handler = NULL;
    self->events_sources[self->events_count - 1].active = 0;

    self->events_count--;
}

// Called during shutdown
static
void free_all_events_source_(struct reactor *self)
{
    for (int i = 0; i < MAX_EVENTS_SRCS; ++i) {
        if (self->events_sources[i].active) {
            free_event_source_(self, i);
        }
    }
}

//--- Reactor interface implementation --------------------------------------------------------

int reactor_add_event_handler(struct reactor *self, struct event_handler *event_handler)
{
    int handler_fd = 0;
    char handler_name[MAX_NAMES];

    assert(self);
    assert(event_handler);

    // Try to add the event handler to the repository
    for (int i = 0; i < MAX_EVENTS_SRCS; ++i) {
        if (!self->events_sources[i].active) {

            // Add event source to the event repository
            self->events_sources[i].handler = event_handler;
            self->events_sources[i].active = 1;

            // Get handler name and file descriptor
            event_handler->get_name(event_handler, handler_name, MAX_NAMES);
            handler_fd = event_handler->get_fd_handle(event_handler);

            // Set pollfd struct
            self->events_fds[i].fd = handler_fd;
            self->events_fds[i].events = POLLIN;

            self->events_count++;

            DBG_PRINT("fred_sys: poll reactor: adding event handler: %s\n",
                        handler_name);

            return 0;
        }
    }

    return -1;
}

void reactor_event_loop(struct reactor *self)
{
    int retval;

    while (1) {
        retval = poll(&self->events_fds[0], self->events_count, -1);
        if (retval < 0) {
            ERROR_PRINT("fred_sys: poll reactor: poll error %s\n", strerror(errno));
            return;
        } else {
            // Handle events
            for (int i = 0; i < self->events_count; ++i) {
                // Ready for a read
                if (self->events_fds[i].revents & POLLIN) {
                    // Handle event
                    retval = self->events_sources[i].handler->handle_event(
                                self->events_sources[i].handler);

                    // Single client error -> detach handler
                    if (retval > 0) {
                        free_event_source_(self, i);

                    // System error -> shutdown
                    } else if (retval < 0) {
                        free_all_events_source_(self);
                        ERROR_PRINT("fred_sys: poll reactor: shutting down event loop");
                        return;
                    }
                }
            }
        }
    }
}

int reactor_init(struct reactor **self)
{
    // Allocate and set everything to zero
    *self = calloc(1, sizeof(**self));
    if (!(*self))
        return -1;

    return 0;
}

void reactor_free(struct reactor *self)
{
    if (!self)
        return;

    free_all_events_source_(self);

    free(self);
}

#endif //USE_POLL
