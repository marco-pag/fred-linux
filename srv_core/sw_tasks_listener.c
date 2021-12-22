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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "sw_task_client.h"
#include "sw_tasks_listener.h"
#include "../utils/dbg_print.h"

// ---------------------- Functions to implement event_handler interface ----------------------

static
int get_fd_handle_(const struct event_handler *self)
{
    struct sw_tasks_listener *sp;

    assert(self);

    sp = (struct sw_tasks_listener *)self;
    return sp->list_sock;
}

static
int handle_event_(struct event_handler *self)
{
    struct sw_tasks_listener *lp;
    struct event_handler *cp;
    int retval;

    assert(self);

    lp = (struct sw_tasks_listener *)self;

    // New connection request from a SW task
    // Create a sw_task_client object
    sw_task_client_init(&cp, lp->list_sock, lp->sys, lp->scheduler, lp->buffctl);

    // And register to the reactor
    retval = reactor_add_event_handler(lp->reactor, cp, REACT_NORMAL_HANDLER, REACT_OWNED);

    return retval;
}

static
void get_name_(const struct event_handler *self, char *msg, int msg_size)
{
    struct sw_tasks_listener *sp;

    assert(msg);
    assert(self);

    sp = (struct sw_tasks_listener *)self;
    snprintf(msg, msg_size, "sw-task listener on fd: %d", sp->list_sock);

}

// Cannot be deallocated through the handler by the reactor
// Must be deallocated manually
static
void free_(struct event_handler *self)
{
    struct sw_tasks_listener *sp;

    if (!self)
        return;

    sp = (struct sw_tasks_listener *)self;

    close(sp->list_sock);
    free(sp);
}

//---------------------------------------------------------------------------------------------

int open_listening_socket_()
{
    int retval;
    int listen_sock;
    struct sockaddr_un serv_addr;
    size_t addr_len;

    // Listening socket
    listen_sock = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listen_sock < 0) {
        ERROR_PRINT("fred_sys: error on opening socket");
        return -1;
    }

    // Initialize and set server addr structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, LIST_SOCK_PATH);

    // Unlink
    unlink(serv_addr.sun_path);

    // Bind
    addr_len = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
    retval = bind(listen_sock, (struct sockaddr *)&serv_addr, addr_len);
    if (retval < 0) {
        ERROR_PRINT("fred_sys: error on bind");
        return -1;
    }

    // One client per hw-task
    retval = listen(listen_sock, MAX_SW_TASKS);
    if (retval < 0) {
        ERROR_PRINT("fred_sys: error on listen");
        return -1;
    }

    return listen_sock;
}

//---------------------------------------------------------------------------------------------

int sw_tasks_listener_init(struct event_handler **self, struct sys_layout *sys,
                            struct reactor *reactor, struct scheduler *scheduler,
                            buffctl_ft *buffctl)
{
    struct sw_tasks_listener *listener;

    assert(sys);
    assert(reactor);
    assert(scheduler);
    assert(buffctl);

    *self = NULL;

    listener = calloc(1, sizeof(*listener));
    if (!listener)
        return -1;

    event_handler_assign_id(&listener->handler);

    // Open socket
    listener->list_sock = open_listening_socket_();
    if (listener->list_sock < 0) {
        ERROR_PRINT("fredsys: software tasks listener: unable to open listening socket\n");
        free(listener);
        return -1;
    }

    // Set properties and methods
    listener->sys = sys;
    listener->reactor = reactor;
    listener->scheduler = scheduler;
    listener->buffctl = buffctl;

    // Event handler interface
    listener->handler.handle_event = handle_event_;
    listener->handler.get_fd_handle = get_fd_handle_;
    listener->handler.get_name = get_name_;
    listener->handler.free = free_;

    *self = &listener->handler;

    return 0;
}

//---------------------------------------------------------------------------------------------
