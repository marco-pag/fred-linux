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

#ifndef SIGNALS_RECV_H_
#define SIGNALS_RECV_H_

#include "event_handler.h"

//---------------------------------------------------------------------------------------------

struct signals_recv {
	// ------------------------//
	struct event_handler handler;	// Handler (inherits)
	// ------------------------//

	int fd;							// Handle (source)
};

//---------------------------------------------------------------------------------------------

static inline
struct event_handler *signals_recv_get_event_handler(struct signals_recv *self)
{
	assert(self);

	return &self->handler;
}

//---------------------------------------------------------------------------------------------

int signals_recv_init(struct signals_recv **self);

void signals_recv_free(struct signals_recv *self);

//---------------------------------------------------------------------------------------------

#endif /* SIGNALS_RECV_H_ */
