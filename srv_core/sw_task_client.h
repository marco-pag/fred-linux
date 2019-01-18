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

#ifndef SW_TASK_CLIENT_H_
#define SW_TASK_CLIENT_H_

#include "../parameters.h"
#include "accel_req.h"
#include "sys_layout.h"
#include "scheduler.h"
#include "hw_task.h"
#include "../shared_user/fred_buff.h"
#include "../srv_support/buffctl.h"

//---------------------------------------------------------------------------------------------

struct sw_task_client {
	// ------------------------//
	struct event_handler handler; 				// Handler (inherits)
	// ------------------------//

	int conn_sock;								// Handle (source)

	enum sw_task_client_state {
		CLIENT_EMPTY,
		CLIENT_READY,
		CLIENT_BUSY
	} state;

	struct hw_task *hw_tasks[MAX_HW_TASKS];		// Associated hw-tasks
	int hw_tasks_count;

	// And data buffers (one set for each hw-task)
	struct fred_buff_if *data_buffs_ifs[MAX_HW_TASKS][MAX_DATA_BUFFS];

	struct scheduler *sched;					// Scheduler state machine
	struct sys_layout *sys;						// System layout

	buffctl_ft *buffctl;						// To allocate buffers (not owning)

	struct accel_req accel_req;					// Max *one* request at time no
												// need to allocate dynamically
};

//---------------------------------------------------------------------------------------------

static inline
struct event_handler *sw_task_client_get_event_handler(struct sw_task_client *self)
{
	assert(self);

	return &self->handler;
}

//---------------------------------------------------------------------------------------------

int sw_task_client_init(struct sw_task_client **self, int list_sock, struct sys_layout *sys,
						struct scheduler *sched, buffctl_ft *buffctl);

int sw_task_client_notify_action(struct sw_task_client *self);

//---------------------------------------------------------------------------------------------

#endif /* SW_TASK_CLIENT_H_ */
