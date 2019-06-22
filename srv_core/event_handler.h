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

#ifndef EVENT_HANDLER_H_
#define EVENT_HANDLER_H_

#include <assert.h>

//---------------------------------------------------------------------------------------------

// handle_event() must return:
// 0 if event has been proper handled
// < 0 in case of system error -> reactor event loop shutdown
// > 0 in case of single handler error (or remove request) -> reactor must detach handler

// In case of error or disconnection request the reactor relies on the
// free() method to (eventually) deallocate the event handler after deregistering

//---------------------------------------------------------------------------------------------

struct event_handler {
	void *self;
	int id;

	int (*get_fd_handle)(void *self);
	int (*handle_event)(void *self);

	void (*get_name)(void *self, char *msg, int msg_size);
	void (*free)(void *self);
};

//---------------------------------------------------------------------------------------------

static inline
void event_handler_assign_id(struct event_handler *self)
{
	static int id_counter = 0;

	assert(self);

	self->id = id_counter++;
}

static inline
int event_handler_get_id(struct event_handler *self)
{
	assert(self);

	return self->id;
}

//---------------------------------------------------------------------------------------------

#endif /* EVENT_HANDLER_H_ */
