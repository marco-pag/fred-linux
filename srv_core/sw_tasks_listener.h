/*
 * Fred for Linux. Experimental support.
 *
 * Copyright (C) 2018, Marco Pagani, ReTiS Lab.
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
#include "scheduler.h"
#include "../srv_support/buffctl.h"


//---------------------------------------------------------------------------------------------

struct sw_tasks_listener {
	// ------------------------//
	struct event_handler handler; 	// Handler (inherits)
	// ------------------------//

	int list_sock;					// Handle (source)

	struct reactor *reactor;		// To add event handler for new client

	struct scheduler *sched;		// To be passed to the client
	struct sys_layout *sys;
	buffctl_ft *buffctl;
};

//---------------------------------------------------------------------------------------------

static inline
struct event_handler *sw_tasks_listener_get_event_handler(struct sw_tasks_listener *self)
{
	assert(self);

	return &self->handler;
}

//---------------------------------------------------------------------------------------------

int sw_tasks_listener_init(struct sw_tasks_listener **self,
							struct sys_layout *sys, struct reactor *reactor,
							struct scheduler *sched, buffctl_ft *buffctl);

void sw_tasks_listener_free(struct sw_tasks_listener *self);

//---------------------------------------------------------------------------------------------

#endif /* SW_TASKS_LISTENER_H_ */
