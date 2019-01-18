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

#include "slot.h"

// ---------------------- Functions to implement event_handler interface ----------------------

static
int get_fd_handle_(void *self)
{
	struct slot *slot;

	assert(self);

	slot = (struct slot *)self;
	return uio_get_fd(slot->ctrl_dev);
}

static
int handle_event_(void *self)
{
	struct slot *slot;

	assert(self);

	slot = (struct slot *)self;

	// Signal the scheduler
	sched_slot_complete(slot->sched, slot->exec_req);

	return 0;
}

static
void get_name_(void *self, char *msg, int msg_size)
{
	struct slot *slot;

	assert(self);

	slot = (struct slot *)self;
	snprintf(msg, msg_size, "slot device %u on fd: %d",
			slot->index,
			uio_get_fd(slot->ctrl_dev));

}

// Cannot be deallocated through the handler by the reactor
// Must be deallocated manually
static
void free_(void *self)
{
	// Empty
}

//---------------------------------------------------------------------------------------------

int slot_init(struct slot **self, int index, const char *dev_name,
				const char *dec_dev_name, struct scheduler *sched)
{
	int retval;

	assert(sched);
	assert(dev_name);
	assert(dec_dev_name);

	// Allocate and set everything to 0
	*self = calloc(1, sizeof(**self));
	if (!(*self))
		return -1;

	event_handler_assign_id(&(*self)->handler);

	// Set slot initial state
	(*self)->index = index;
	(*self)->state = SLOT_BLANK;
	(*self)->sched = sched;

	// Event handler interface
	(*self)->handler.self = *self;
	(*self)->handler.handle_event = handle_event_;
	(*self)->handler.get_fd_handle = get_fd_handle_;
	(*self)->handler.get_name = get_name_;
	(*self)->handler.free = free_;

	// Init UIO device driver for slot and decoupler
	// Names must match device tree names
	retval = slot_drv_dev_init(&(*self)->ctrl_dev, dev_name);
	if (retval < 0) {
		free(*self);
		return -1;
	}

	// Decoupler
	retval = decoup_drv_dev_init(&(*self)->dec_dev, dec_dev_name);
	if (retval < 0) {
		slot_drv_dev_free((*self)->ctrl_dev);
		free(*self);
		return -1;
	}

	return 0;
}

void slot_free(struct slot *self)
{
	assert(self);

	if (self->ctrl_dev)
		slot_drv_dev_free(self->ctrl_dev);

	if (self->dec_dev)
		decoup_drv_dev_free(self->dec_dev);

	free(self);
}
