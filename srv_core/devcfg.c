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

#include <stdlib.h>
#include <stdio.h>


#include "accel_req.h"
#include "slot.h"
#include "devcfg.h"

#include "../hw_support/devcfg_drv_fpga_mgr.h"

// ---------------------- Functions to implement event_handler interface ----------------------

static
int get_fd_handle_(const struct event_handler *self)
{
    struct devcfg *dev;

    assert(self);

    dev = (struct devcfg *)self;
    return devcfg_drv_get_fd(dev->drv);
}

static
int handle_event_(struct event_handler *self)
{
    struct devcfg *dev;
    int retval;

    assert(self);

    dev = (struct devcfg *)self;
    retval = scheduler_rcfg_complete(dev->scheduler, dev->current_rcfg_req);

    return retval;
}

static
void get_name_(const struct event_handler *self, char *msg, int msg_size)
{
    struct devcfg *dev;

    assert(self);
    assert(msg);

    dev = (struct devcfg *)self;
    snprintf(msg, msg_size, "devcfg on fd: %d", devcfg_drv_get_fd(dev->drv));
}

static
void free_(struct event_handler *self)
{
    struct devcfg *dev;

    if (!self)
        return;

    dev = (struct devcfg *)self;

    if (dev->drv)
        devcfg_drv_free(dev->drv);

    free(dev);
}

//---------------------------------------------------------------------------------------------

int devcfg_init(struct devcfg **self, const struct sys_hw_config *hw_config)
{
    int retval;
    enum sys_devcfg_type devcfg_type;

    *self = calloc(1, sizeof(**self));
    if (!(*self))
        return -1;

    event_handler_assign_id(&(*self)->handler);

    // Set properties and methods
    (*self)->state = DEVCFG_INIT;

    // Event handler interface
    (*self)->handler.handle_event = handle_event_;
    (*self)->handler.get_fd_handle = get_fd_handle_;
    (*self)->handler.get_name = get_name_;
    (*self)->handler.free = free_;

    // Initialize FPGA driver
    devcfg_type = sys_hw_config_get_devcfg_type(hw_config);

    switch (devcfg_type) {
        case SYS_DEVCFG_FPGA_MGR:
        case SYS_DEVCFG_NULL:       // To be implemented
        default:
            retval = devcfg_drv_fpga_mgr_init(&(*self)->drv);
            break;
    }

    if (retval) {
        free(*self);
        return -1;
    }

    return 0;
}

void devcfg_attach_scheduler(struct devcfg *self, struct scheduler *scheduler)
{
    assert(self);
    assert(scheduler);
    assert(self->state == DEVCFG_INIT);

    self->scheduler = scheduler;
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
    return devcfg_drv_start_rcfg(self->drv, accel_req_get_phy_bit(request));
}

uint64_t devcfg_clear_evt(struct devcfg *self)
{
    assert(self);
    assert(self->state == DEVCFG_PROG);

    self->state = DEVCFG_IDLE;

    return devcfg_drv_after_rcfg(self->drv);
}

