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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "fred_sys.h"
#include "devcfg.h"
#include "signals_recv.h"
#include "../utils/logger.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

static const char fred_logo[] =
"\n"
"_|_|_|_|  _|_|_|    _|_|_|_|  _|_|_|    \n"
"_|        _|    _|  _|        _|    _|  \n"
"_|_|_|    _|_|_|    _|_|_|    _|    _|  \n"
"_|        _|    _|  _|        _|    _|  \n"
"_|        _|    _|  _|_|_|_|  _|_|_|    \n"
"\n";

//---------------------------------------------------------------------------------------------

struct fred_sys {
	// Epoll event reactor (own)
	struct reactor *reactor;

	// Signal handler
	struct signals_recv *signals_receiver;

	// System layout: partition, slots, and hw-tasks (own)
	struct sys_layout *layout;

	// Buffer allocator kernel module (own)
	buffctl_ft *buffctl;

	// Reconfiguration device (own)
	struct devcfg *devcfg;

	// Listener for sw-tasks
	struct sw_tasks_listener *sw_tasks_listener;

	// FRED scheduler component
	struct scheduler *scheduler;
};

//---------------------------------------------------------------------------------------------

int fred_sys_init(struct fred_sys **self, const char *arch_file,
				  const char *hw_tasks_file)
{
	int retval;

	*self = calloc(1, sizeof(**self));
	if (!(*self))
		return -1;

	DBG_PRINT(fred_logo);

	// Initialize logger
	retval = logger_init();
	if (retval) {
		ERROR_PRINT("fred_sys: error while initializing logger\n");
		goto error_logger;
	}

	// Init event demultiplexer interface
	retval = reactor_init(&(*self)->reactor);
	if (retval) {
		ERROR_PRINT("fred_sys: error while initializing event reactor\n");
		goto error_reactor;
	}

	// Open file descriptor based signal handler
	retval = signals_recv_init(&(*self)->signals_receiver);
	if (retval) {
		ERROR_PRINT("fred_sys: error while initializing fd for receiving signals\n");
		goto error_signals_recv;
	}

	// Open kernel module interface file
	retval = buffctl_open(&(*self)->buffctl, NULL);
	if (retval) {
		ERROR_PRINT("fred_sys: unable to open buffctl device\n");
		goto error_buffctl;
	}

	// Open reconfiguration device
	retval = devcfg_init(&(*self)->devcfg);
	if (retval) {
		ERROR_PRINT("fred_sys: unable to open devcfg device\n");
		goto error_devcfg;
	}

	// Initalize scheduler
	retval = sched_init(&(*self)->scheduler, (*self)->devcfg);
	if (retval) {
		ERROR_PRINT("fred_sys: error while initializing scheduler\n");
		goto error_sched;
	}

	// Link scheduler to devcfg
	devcfg_attach_scheduler((*self)->devcfg, (*self)->scheduler);

	// Initialize partitions, slots, and hw-tasks
	retval = sys_layout_init(&(*self)->layout, arch_file, hw_tasks_file,
							(*self)->scheduler, (*self)->buffctl);
	if (retval) {
		ERROR_PRINT("fred_sys: error while initializing system layout\n");
		goto error_sys_layout;
	}

	// For debugging purposes
	sys_layout_print((*self)->layout);

	// Create sw-task listener
	retval = sw_tasks_listener_init(&(*self)->sw_tasks_listener, (*self)->layout,
									(*self)->reactor, (*self)->scheduler, (*self)->buffctl);
	if (retval) {
		ERROR_PRINT("fred_sys: error while initializing sw-task listener\n");
		goto error_sw_task_listner;
	}

	return 0;

// Error handling chain
error_sw_task_listner:
	sys_layout_free((*self)->layout);
error_sys_layout:
	sched_free((*self)->scheduler);
error_sched:
	devcfg_free((*self)->devcfg);
error_devcfg:
	buffctl_close((*self)->buffctl);
error_buffctl:
	signals_recv_free((*self)->signals_receiver);
error_signals_recv:
	reactor_free((*self)->reactor);
error_reactor:
	logger_free();
error_logger:
	free(*self);
	return -1;
}

void fred_sys_free(struct fred_sys *self)
{
	if (!self)
		return;

	// Will release all registered clients using the
	// free method of the handler
	if (self->reactor)
		reactor_free(self->reactor);

	if (self->sw_tasks_listener)
		sw_tasks_listener_free(self->sw_tasks_listener);

	if (self->scheduler)
		sched_free(self->scheduler);

	if (self->signals_receiver)
		signals_recv_free(self->signals_receiver);

	if (self->devcfg)
		devcfg_free(self->devcfg);

	if (self->layout)
		sys_layout_free(self->layout);

	if (self->buffctl)
		buffctl_close(self->buffctl);

	logger_free();

	free(self);
}

int fred_sys_run(struct fred_sys *self)
{
	int retval;

	assert(self);

	// Register file based signal handler
	retval = reactor_add_event_handler(self->reactor,
									signals_recv_get_event_handler(self->signals_receiver));
	if (retval) {
		ERROR_PRINT("fred_sys: error while registering signals handler\n");
		return -1;
	}


	// Register reconfiguration device event handler
	// (and source) to the reactor
	retval = reactor_add_event_handler(self->reactor, devcfg_get_event_handler(self->devcfg));
	if (retval) {
		ERROR_PRINT("fred_sys: error while registering reconfiguration device handler\n");
		return -1;
	}

	// Register all slots of all partitions
	retval = sys_layout_register_slots(self->layout, self->reactor);
	if (retval) {
		ERROR_PRINT("fred_sys: error while registering slots handler\n");
		return -1;
	}

	// Register sw-task listener
	retval = reactor_add_event_handler(self->reactor,
								sw_tasks_listener_get_event_handler(self->sw_tasks_listener));
	if (retval) {
		ERROR_PRINT("fred_sys: error while registering sw-task listener handler\n");
		return -1;
	}

	// Start event loop
	reactor_event_loop(self->reactor);

	return 0;
}
