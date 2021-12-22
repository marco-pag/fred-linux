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
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "reactor.h"
#include "event_handler.h"
#include "../parameters.h"
#include "../utils/dbg_print.h"

#ifndef USE_POLL

//---------------------------------------------------------------------------------------------

// Epoll implementation of the reactor

//---------------------------------------------------------------------------------------------

// Internal event handler wrapper
struct event_source_ {
    int active;
    enum react_handler_ownership ownership;
    struct event_handler *handler;
};

//---------------------------------------------------------------------------------------------

struct reactor {
    // Epoll file descriptor
    int ep_fd;

    // Events handlers repository
    struct event_source_ events_sources[MAX_EVENTS_SRCS];
};

//--- Private methods -------------------------------------------------------------------------

static
void free_event_source_(struct reactor *self, struct event_source_ *event_src)
{
    int handler_fd;
    char handler_name[MAX_NAMES];

    // Remove from epoll
    handler_fd = event_handler_get_fd_handle(event_src->handler);
    epoll_ctl(self->ep_fd, EPOLL_CTL_DEL, handler_fd, NULL);

    // Get handler name for print
    event_handler_get_name(event_src->handler, handler_name, MAX_NAMES);

    DBG_PRINT("fred_sys: epoll reactor: removing event handler %s\n", handler_name);

    // Free the event handler object
    event_handler_free(event_src->handler);

    // Clear repository flag
    event_src->handler = NULL;
    event_src->active = 0;
}

static
void free_all_events_source_(struct reactor *self)
{
    for (int i = 0; i < MAX_EVENTS_SRCS; ++i) {
        if (self->events_sources[i].active &&
            self->events_sources[i].ownership == REACT_OWNED) {
                free_event_source_(self, &self->events_sources[i]);
        }
    }
}

//--- Reactor interface implementation --------------------------------------------------------

int reactor_add_event_handler(struct reactor *self, struct event_handler *event_handler,
                                enum react_handler_mode handler_mode,
                                enum react_handler_ownership handler_ownership)
{
    int retval;
    int handler_fd = 0;
    char handler_name[MAX_NAMES];

    // For epoll
    struct epoll_event epoll_event;

    assert(self);
    assert(event_handler);

    // Try to add the event handler to the repository
    for (int i = 0; i < MAX_EVENTS_SRCS; ++i) {
        if (!self->events_sources[i].active) {

            // Add event source to the event repository
            self->events_sources[i].handler = event_handler;
            self->events_sources[i].ownership = handler_ownership;
            self->events_sources[i].active = 1;

            // Fill the epoll event structure and link the event handler wrapper
            switch (handler_mode) {
                case REACT_PRI_HANDLER:
                    epoll_event.events = EPOLLERR | EPOLLPRI;
                    break;
                case REACT_NORMAL_HANDLER:
                default:
                    epoll_event.events = EPOLLIN;
                    break;
            }

            epoll_event.data.ptr = &self->events_sources[i];

            // Get handler name and file descriptor
            event_handler_get_name(event_handler, handler_name, MAX_NAMES);
            handler_fd = event_handler_get_fd_handle(event_handler);

            DBG_PRINT("fred_sys: epoll reactor: adding event handler: %s\n",
                        handler_name);

            // Add to epoll
            retval = epoll_ctl(self->ep_fd, EPOLL_CTL_ADD, handler_fd, &epoll_event);
            if (retval < 0) {
                ERROR_PRINT("fred_sys: epoll reactor: epoll_ctl: could not add event!\n");
                return -1;
            }

            return 0;
        }
    }

    return -1;
}

void reactor_event_loop(struct reactor *self)
{
    int retval;
    struct event_source_ *event_src;

    // For epoll
    struct epoll_event epoll_events[MAX_EVENTS_SRCS];
    int events_count;

    while (1) {
        // Wait for events
        events_count = epoll_wait(self->ep_fd, epoll_events, MAX_EVENTS_SRCS, -1);
        if (events_count < 0) {
            ERROR_PRINT("fred_sys: epoll reactor: epoll_wait error %s\n", strerror(errno));
            goto exit_clear;
        }

        // Process active events
        for (int i = 0; i < events_count; ++i) {

            // Get event handle
            event_src = (struct event_source_ *)epoll_events[i].data.ptr;

            // Check event class
            if (epoll_events[i].events & EPOLLRDHUP) {
                ERROR_PRINT("fred_sys: EPOLLRDHUP, connection closed\n");
                free_event_source_(self, event_src);
                continue;

            } else if ((epoll_events[i].events & EPOLLERR) &&
                        !(epoll_events[i].events & EPOLLPRI)) {
                ERROR_PRINT("fred_sys: epoll reactor: epoll_wait error\n");
                goto exit_clear;
            }

            assert(event_src);

            // Handle event
            retval = event_handler_handle_event(event_src->handler);

            // Single client error -> detach handler
            if (retval > 0) {
                free_event_source_(self, event_src);
                continue;

            // System error -> shutdown
            } else if (retval < 0) {
                goto exit_clear;
            }
        }
    }

// Still the most straightforward way to exit from nested loops
// without using cumbersome logic conditions and unnecessary extra variables
exit_clear:
    ERROR_PRINT("fred_sys: epoll reactor: shutting down event loop\n");
    free_all_events_source_(self);
    return;
}

int reactor_init(struct reactor **self)
{
    // Allocate and set everything to zero
    *self = calloc(1, sizeof(**self));
    if (!(*self))
        return -1;

    // Create epoll instance
    (*self)->ep_fd = epoll_create1(0);
    if ((*self)->ep_fd < 0) {
        ERROR_PRINT("fred_sys: epoll reactor: epoll_create error\n");
        free(*self);
        return -1;
    }

    return 0;
}

void reactor_free(struct reactor *self)
{
    if (!self)
        return;

    free_all_events_source_(self);
    close(self->ep_fd);

    free(self);
}

#endif //USE_POLL
