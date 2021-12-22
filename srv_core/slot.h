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

#ifndef SLOT_H_
#define SLOT_H_

#include <assert.h>
#include <stdint.h>

#include "event_handler.h"
#include "accel_req.h"
#include "hw_task.h"
#include "scheduler.h"

#include "../hw_support/sys_hw_config.h"
#include "../hw_support/decoup_drv.h"
#include "../hw_support/slot_drv.h"


//---------------------------------------------------------------------------------------------
//
//                  |--------(skip reconfiguration)---------| |-(timeout)-|
//                  |                                       V |           V
// SLOT_BLANK -> SLOT_RSRV -> SLOT_RCFG -> SLOT_READY -> SLOT_EXEC -> SLOT_IDLE
//                  ^                                                     |
//                  |-----------------------------------------------------|
//
//---------------------------------------------------------------------------------------------

struct slot {
    // ------------------------//
    struct event_handler handler;    // Handler interface
    // ------------------------//

    // Each slot contains ad accelerator
    // (from which one can get the handle for event_source)
    // and it's paired with a decoupler
    struct slot_drv *slot_dev;      // Handle
    struct decoup_drv *dec_dev;

    enum slot_state {
        SLOT_BLANK,                 // Not configured
        SLOT_RSRV,                  // Reserved to an hw-task instance
        SLOT_RCFG,                  // Under reconfiguration
        SLOT_READY,                 // Ready (re-enabled) after reconfiguration
        SLOT_EXEC,                  // Executing
        SLOT_IDLE                   // Idle (configured with an hw-task)
    } state;

    int index;

    struct scheduler *scheduler;

    // Holds the request associated to the
    // current executing hw-task
    struct accel_req *exec_req;

    // Currently configured Hw-task
    // Consistent when in SLOT_IDLE state
    struct hw_task *hw_task;
};

//---------------------------------------------------------------------------------------------

static inline
struct event_handler *slot_get_event_handler(struct slot *self)
{
    assert(self);

    return &self->handler;
}

//---------------------------------------------------------------------------------------------

static inline
int slot_get_index(const struct slot *self)
{
    assert(self);

    return self->index;
}

static inline
int slot_is_available(const struct slot *self)
{
    assert(self);

    return self->state == SLOT_IDLE || self->state == SLOT_BLANK;
}

static inline
int slot_match_hw_task(const struct slot *self, const struct hw_task *hw_task)
{
    assert(self);

    if (self->state != SLOT_IDLE)
        return 0;

    return hw_task_get_id(self->hw_task) == hw_task_get_id(hw_task);
}

static inline
void slot_set_hw_task(struct slot *self, struct hw_task *hw_task)
{
    assert(self);
    assert(hw_task);
    assert(self->state == SLOT_RCFG);

    self->hw_task = hw_task;
}

// For testing purposes
static inline
int slot_check_hw_task_consistency(const struct slot *self)
{
    assert(self);

    return slot_drv_get_id(self->slot_dev) == hw_task_get_id(self->hw_task);
}

//---------------------------------------------------------------------------------------------

static inline
void slot_set_reserved(struct slot *self)
{
    assert(self);
    assert(self->state == SLOT_IDLE || self->state == SLOT_BLANK);

    self->state = SLOT_RSRV;
}

static inline
void slot_prepare_for_rcfg(struct slot *self)
{
    assert(self);
    assert(self->state == SLOT_RSRV);

    decoup_drv_decouple(self->dec_dev);
    self->state = SLOT_RCFG;
}

static inline
void slot_reinit_after_rcfg(struct slot *self)
{
    assert(self);
    assert(self->state == SLOT_RCFG);

    self->state = SLOT_READY;
    decoup_drv_couple(self->dec_dev);
    slot_drv_after_rcfg(self->slot_dev);
}

static inline
int slot_start_compute(struct slot *self, struct accel_req *exec_req)
{
    int retval;

    assert(self);
    assert(exec_req);
    assert(self->state == SLOT_READY || self->state == SLOT_RSRV);

    // Bind request to the slot
    self->exec_req = exec_req;
    self->state = SLOT_EXEC;

    // And start
    retval = slot_drv_start_compute(self->slot_dev,
                                    accel_req_get_args(exec_req),
                                    accel_req_get_args_size(exec_req));

    return retval;
}

static inline
void slot_disable_after_timeout(struct slot *self)
{
    assert(self);
    assert(self->state == SLOT_EXEC);

    // Suspend the slot until the next reconfiguration
    decoup_drv_decouple(self->dec_dev);

    // Set blank to avoid reuse and force a new reconfiguration
    self->state = SLOT_BLANK;
}

static inline
void slot_clear_after_compute(struct slot *self)
{
    assert(self);
    assert(self->state == SLOT_EXEC);

    slot_drv_after_compute(self->slot_dev);

    self->state = SLOT_IDLE;
}

//---------------------------------------------------------------------------------------------

int slot_init(struct slot **self, const struct sys_hw_config *hw_config, int index,
                const char *dev_name, const char *dec_dev_name, struct scheduler *scheduler);

#endif /* SLOT_H_ */
