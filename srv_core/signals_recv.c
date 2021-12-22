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

#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "signals_recv.h"
#include "../utils/dbg_print.h"

// ---------------------- Functions to implement event_handler interface ----------------------

static
int get_fd_handle_(const struct event_handler *self)
{
    struct signals_recv *recv;

    assert(self);

    recv = (struct signals_recv *)self;
    return recv->fd;
}

static
int handle_event_(struct event_handler *self)
{
    struct signals_recv *recv;
    struct signalfd_siginfo fdsi;
    int retval;

    assert(self);

    recv = (struct signals_recv *)self;

    // Receive signal by reading
    retval = read(recv->fd, &fdsi, sizeof(struct signalfd_siginfo));
    if (retval != sizeof(struct signalfd_siginfo)) {
        ERROR_PRINT("fred_sys: Error while reading for a signal.\n");
        return -1;
    }

    // Handle signals
    switch (fdsi.ssi_signo) {
    case SIGINT:
        DBG_PRINT("fred_sys: received signal SIGINT, terminating.\n");
        retval = -1;
        break;

    case SIGQUIT:
        DBG_PRINT("fred_sys: received signal SIGQUIT, terminating.\n");
        retval = -1;
        break;

    case SIGTERM:
        DBG_PRINT("fred_sys: received signal SIGTERM, terminating.\n");
        retval = -1;
        break;

    default:
        DBG_PRINT("fred_sys: received unexpected signal, terminating.\n");
        retval = -1;
        break;
    }

    return retval;
}

static
void get_name_(const struct event_handler *self, char *msg, int msg_size)
{
    struct signals_recv *recv;

    assert(self);

    recv = (struct signals_recv *)self;
    snprintf(msg, msg_size, "signals handler on fd: %d", recv->fd);
}

static
void free_(struct event_handler *self)
{
    struct signals_recv *recv;

    if (!self)
        return;

    recv = (struct signals_recv *)self;

    close(recv->fd);
    free(recv);
}

//---------------------------------------------------------------------------------------------

int signals_recv_init(struct event_handler **self)
{
    struct signals_recv *recv;
    int retval;
    sigset_t mask;

    *self = NULL;

    // Allocate and set everything to 0
    recv = calloc(1, sizeof (*recv));
    if (!recv)
        return -1;

    event_handler_assign_id(&recv->handler);

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);

    // Block signals form being raised normally
    retval = sigprocmask(SIG_BLOCK, &mask, NULL);
    if (retval) {
        ERROR_PRINT("fred_sys: unable to mask signals. Error: %s\n", strerror(errno));
        free(recv);
        return -1;
    }

    recv->fd = signalfd(-1, &mask, SFD_NONBLOCK);
    if (recv->fd < 0) {
        ERROR_PRINT("fred_sys: unable create signals fd. Error: %s\n", strerror(errno));
        free(recv);
        return -1;
    }

    // Event handler interface
    recv->handler.handle_event = handle_event_;
    recv->handler.get_fd_handle = get_fd_handle_;
    recv->handler.get_name = get_name_;
    recv->handler.free = free_;

    *self = &recv->handler;

    return 0;
}
