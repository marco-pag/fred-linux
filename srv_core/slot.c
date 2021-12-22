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

#include "../hw_support/sys_hw_config.h"
#include "../hw_support/slot_drv_null.h"
#include "../hw_support/slot_drv_master.h"
#include "../hw_support/decoup_drv_xil.h"

#include "slot.h"

// ---------------------- Functions to implement event_handler interface ----------------------

static
int get_fd_handle_(const struct event_handler *self)
{
    struct slot *slot;

    assert(self);

    slot = (struct slot *)self;
    return slot_drv_get_fd(slot->slot_dev);
}

static
int handle_event_(struct event_handler *self)
{
    struct slot *slot;
    int retval;

    assert(self);

    slot = (struct slot *)self;

    // Signal the scheduler
    retval = scheduler_slot_complete(slot->scheduler, slot->exec_req);

    return retval;
}

static
void get_name_(const struct event_handler *self, char *msg, int msg_size)
{
    struct slot *slot;

    assert(self);

    slot = (struct slot *)self;
    snprintf(msg, msg_size, "slot device %u on fd: %d",
            slot->index,
            slot_drv_get_fd(slot->slot_dev));
}

static
void free_(struct event_handler *self)
{
    struct slot *slot;

    if (!self)
        return;

    slot = (struct slot *)self;

    if (slot->slot_dev)
        slot_drv_free(slot->slot_dev);

    if (slot->dec_dev)
        decoup_drv_free(slot->dec_dev);

    free(slot);
}

//---------------------------------------------------------------------------------------------

int slot_init(struct slot **self, const struct sys_hw_config *hw_config, int index,
                const char *dev_name, const char *dec_dev_name, struct scheduler *scheduler)
{
    int retval;
    enum sys_slot_type slot_type;

    assert(scheduler);
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
    (*self)->scheduler = scheduler;

    // Event handler interface
    (*self)->handler.handle_event = handle_event_;
    (*self)->handler.get_fd_handle = get_fd_handle_;
    (*self)->handler.get_name = get_name_;
    (*self)->handler.free = free_;

    // Initialize slots drivers
    slot_type = sys_hw_config_get_slot_type(hw_config);

    switch (slot_type) {
        case SYS_SLOT_MASTER:
            // UIO device names must match device tree names
            retval = slot_drv_master_init(&(*self)->slot_dev, dev_name);
            break;
        case SYS_SLOT_NULL:
        default:
            retval = slot_drv_null_init(&(*self)->slot_dev, dev_name);
            break;
    }

    if (retval < 0) {
        free(*self);
        return -1;
    }

    // Decoupler
    // UIO device names must match device tree names
    retval = decoup_drv_xil_init(&(*self)->dec_dev, dec_dev_name);
    if (retval < 0) {
        slot_drv_free((*self)->slot_dev);
        free(*self);
        return -1;
    }

    return 0;
}
