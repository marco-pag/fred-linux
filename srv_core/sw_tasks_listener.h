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

#ifndef SW_TASKS_LISTENER_H_
#define SW_TASKS_LISTENER_H_

#include <assert.h>

#include "../parameters.h"
#include "reactor.h"
#include "../srv_support/buffctl.h"
#include "scheduler.h"


//---------------------------------------------------------------------------------------------

struct sw_tasks_listener {
    // ------------------------//
    struct event_handler handler;   // Handler interface
    // ------------------------//

    int list_sock;                  // Handle

    struct reactor *reactor;        // To add event handler for new client

    struct scheduler *scheduler;    // To be passed to the client
    struct sys_layout *sys;
    buffctl_ft *buffctl;
};

//---------------------------------------------------------------------------------------------

int sw_tasks_listener_init(struct event_handler **self, struct sys_layout *sys,
                            struct reactor *reactor, struct scheduler *scheduler,
                            buffctl_ft *buffctl);

//---------------------------------------------------------------------------------------------

#endif /* SW_TASKS_LISTENER_H_ */
