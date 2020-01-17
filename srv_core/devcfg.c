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

#include <stdlib.h>
#include <stdio.h>

#include "devcfg.h"
#include "accel_req.h"
#include "slot.h"

// ---------------------- Functions to implement event_handler interface ----------------------

static
int get_fd_handle_(void *self)
{
	struct devcfg *dev;

	assert(self);

	dev = (struct devcfg *)self;
	return devcfg_drv_get_fd(dev->drv);
}

static
int handle_event_(void *self)
{
	struct devcfg *dev;

	assert(self);

	dev = (struct devcfg *)self;
	sched_rcfg_complete(dev->sched, dev->current_rcfg_req);

	return 0;
}

static
void get_name_(void *self, char *msg, int msg_size)
{
	struct devcfg *dev;

	assert(self);
	assert(msg);

	dev = (struct devcfg *)self;
	snprintf(msg, msg_size, "devcfg on fd: %d", devcfg_drv_get_fd(dev->drv));
}

// Cannot be deallocated through the handler by the reactor
// Must be deallocated manually
static
void free_(void *self)
{
	// Empty
}

//---------------------------------------------------------------------------------------------

int devcfg_init(struct devcfg **self)
{
	int retval;

	*self = calloc(1, sizeof(**self));
	if (!(*self))
		return -1;

	event_handler_assign_id(&(*self)->handler);

	retval = devcfg_drv_init(&(*self)->drv);
	if (retval) {
		free(*self);
		return -1;
	}

	// Set properties and methods
	(*self)->state = DEVCFG_INIT;

	// Event handler interface
	(*self)->handler.self = *self;
	(*self)->handler.handle_event = handle_event_;
	(*self)->handler.get_fd_handle = get_fd_handle_;
	(*self)->handler.get_name = get_name_;
	(*self)->handler.free = free_;

	return 0;
}

void devcfg_free(struct devcfg *self)
{
	if (!self)
		return;

	if (self->drv)
		devcfg_drv_free(self->drv);

	free(self);
}

void devcfg_attach_scheduler(struct devcfg *self, struct scheduler *sched)
{
	assert(self);
	assert(sched);
	assert(self->state == DEVCFG_INIT);

	self->sched = sched;
	self->state = DEVCFG_IDLE;
}

int devcfg_start_prog(struct devcfg *self, struct accel_req *request)
{

	assert(self);
	assert(request);
	assert(self->state == DEVCFG_IDLE);

	self->state = DEVCFG_PROG;
	self->current_rcfg_req = request;

	// Bind hw-task to the slot
	slot_set_hw_task(accel_req_get_slot(request), accel_req_get_hw_task(request));

	// And start reconfiguration
	return devcfg_drv_start_prog(self->drv, accel_req_get_phy_bit(request));
}

int64_t devcfg_clear_evt(struct devcfg *self)
{
	assert(self);
	assert(self->state == DEVCFG_PROG);

	self->state = DEVCFG_IDLE;

	return devcfg_drv_clear_evt(self->drv);
}

