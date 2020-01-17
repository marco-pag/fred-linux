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

#ifndef DEVCFG_H_
#define DEVCFG_H_

#include <assert.h>
#include <stdint.h>

#include "event_handler.h"
#include "scheduler.h"
#include "../hw_support/devcfg_drv.h"

//---------------------------------------------------------------------------------------------

struct accel_req;

//---------------------------------------------------------------------------------------------

// NOTE:
// Event handler wrapped around a stripped down version of
// the reconfiguration driver code.
// Will be modified in future releases.

//---------------------------------------------------------------------------------------------

struct devcfg {
	// ------------------------//
	struct event_handler handler; 	// Handler (inherits)
	// ------------------------//

	devcfg_drv *drv;				// Handle (source)

	enum {DEVCFG_INIT ,DEVCFG_IDLE, DEVCFG_PROG} state;

	// Pointer to the (single) request under reconfiguration
	struct accel_req *current_rcfg_req;

	struct scheduler *sched;
};

//---------------------------------------------------------------------------------------------

static inline
struct event_handler *devcfg_get_event_handler(struct devcfg *self)
{
	assert(self);

	return &self->handler;
}

//---------------------------------------------------------------------------------------------

static inline
int devcfg_is_idle(const struct devcfg *self)
{
	assert(self);

	return self->state == DEVCFG_IDLE;
}

static inline
int devcfg_is_prog(const struct devcfg *self)
{
	assert(self);

	return self->state == DEVCFG_PROG;
}

static inline
int devcfg_get_fd(const struct devcfg *self)
{
	assert(self);

	return devcfg_drv_get_fd(self->drv);
}

//---------------------------------------------------------------------------------------------

int devcfg_init(struct devcfg **self);

void devcfg_free(struct devcfg *self);

// To avoid cyclic initialization dependency between devcfg and scheduler
void devcfg_attach_scheduler(struct devcfg *self, struct scheduler *sched);

int devcfg_start_prog(struct devcfg *self, struct accel_req *request);

int64_t devcfg_clear_evt(struct devcfg *self);

//---------------------------------------------------------------------------------------------

#endif /* DEVCFG_H_ */
