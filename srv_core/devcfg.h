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

#ifndef DEVCFG_H_
#define DEVCFG_H_

#include <assert.h>
#include <stdint.h>

#include "event_handler.h"
#include "accel_req.h"
#include "../hw_support/sys_hw_config.h"
#include "../hw_support/devcfg_drv.h"
#include "scheduler.h"

//---------------------------------------------------------------------------------------------

struct devcfg {
    // ------------------------//
    struct event_handler handler;       // Handler interface
    // ------------------------//

    struct devcfg_drv *drv;             // Handle

    enum {DEVCFG_INIT, DEVCFG_IDLE, DEVCFG_PROG} state;

    // Pointer to the (single) request under reconfiguration
    struct accel_req *current_rcfg_req;

    struct scheduler *scheduler;
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

//---------------------------------------------------------------------------------------------

int devcfg_init(struct devcfg **self, const struct sys_hw_config *hw_config);

// To avoid cyclic initialization dependency between devcfg and scheduler
void devcfg_attach_scheduler(struct devcfg *self, struct scheduler *scheduler);

int devcfg_start_prog(struct devcfg *self, struct accel_req *request);

// Returns reconfiguration time in us
uint64_t devcfg_clear_evt(struct devcfg *self);

//---------------------------------------------------------------------------------------------

#endif /* DEVCFG_H_ */
