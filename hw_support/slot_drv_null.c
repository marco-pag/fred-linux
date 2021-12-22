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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "slot_drv_null.h"
#include "../utils/fd_utils.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

static
uint32_t slot_drv_null_get_id_(const struct slot_drv *self)
{
    assert(self);

    return 0U;
}

static
int slot_drv_null_get_fd_(const struct slot_drv *self)
{
    struct slot_drv_null *null_drv;

    assert(self);

    null_drv = (struct slot_drv_null *)self;

    return null_drv->out_fd;
}

static
void slot_drv_null_before_rcfg_(struct slot_drv *self)
{
    assert(self);

    // Empty, no actions required
}

static
void slot_drv_null_after_rcfg_(struct slot_drv *self)
{
    assert(self);

    // Empty, no actions required
}

int slot_drv_null_start_compute_(struct slot_drv *self, const uintptr_t *args,
                                    int args_size)
{
    struct slot_drv_null *null_drv;

    assert(self);

    null_drv = (struct slot_drv_null *)self;

    return fd_utils_byte_write(null_drv->in_fd);
}

void slot_drv_null_after_compute_(struct slot_drv *self)
{
    struct slot_drv_null *null_drv;

    assert(self);

    null_drv = (struct slot_drv_null *)self;

    fd_utils_byte_read(null_drv->out_fd);
}

void slot_drv_null_wait_for_compl_(const struct slot_drv *self)
{
    assert(self);

    // Empty, no actions required
}

void slot_drv_null_free_(struct slot_drv *self)
{
    struct slot_drv_null *null_drv;

    if (!self)
        return;

    null_drv = (struct slot_drv_null *)self;

    close(null_drv->in_fd);
    close(null_drv->out_fd);

    free(null_drv);
}

//---------------------------------------------------------------------------------------------

int slot_drv_null_init(struct slot_drv **self, const char *dev_name)
{
    struct slot_drv_null *null_drv;
    int retval;

    assert(self);

    *self = NULL;

    null_drv = calloc(1, sizeof (*null_drv));
    if (!null_drv)
        return -1;

    // Slot interface
    null_drv->slot_drv.get_id = slot_drv_null_get_id_;
    null_drv->slot_drv.get_fd = slot_drv_null_get_fd_;
    null_drv->slot_drv.before_rcfg = slot_drv_null_before_rcfg_;
    null_drv->slot_drv.after_rcfg = slot_drv_null_after_rcfg_;
    null_drv->slot_drv.start_compute = slot_drv_null_start_compute_;
    null_drv->slot_drv.after_compute = slot_drv_null_after_compute_;
    null_drv->slot_drv.wait_for_compl = slot_drv_null_wait_for_compl_;
    null_drv->slot_drv.free = slot_drv_null_free_;


    retval = fd_utils_create_socket_pair(&null_drv->in_fd, &null_drv->out_fd);
    if (retval) {
        ERROR_PRINT("fred_sys: null_drv: cannot create socket pair\n");
        free(null_drv);
        return -1;
    }

    retval = fd_utils_set_fd_nonblock(null_drv->out_fd);
    if (retval) {
        ERROR_PRINT("fred_sys: null_drv: error on set non-block\n");
        free(null_drv);
        return -1;
    }

    *self = &null_drv->slot_drv;

    return 0;
}


