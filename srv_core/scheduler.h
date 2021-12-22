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

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "../parameters.h"

//---------------------------------------------------------------------------------------------

struct accel_req;

//---------------------------------------------------------------------------------------------
// Scheduler interface

struct scheduler {

    int (*push_accel_req)(struct scheduler *self, struct accel_req *request);

    int (*rcfg_complete)(struct scheduler *self, struct accel_req *request);

    int (*slot_complete)(struct scheduler *self, struct accel_req *request);

    int (*slot_timeout)(struct scheduler *self, struct accel_req *request);

    void (*free)(struct scheduler *self);
};

//---------------------------------------------------------------------------------------------

static inline
int scheduler_push_accel_req(struct scheduler *self, struct accel_req *request)
{
    assert(self);

    return self->push_accel_req(self, request);
}

static inline
int scheduler_rcfg_complete(struct scheduler *self, struct accel_req *request)
{
    assert(self);

    return self->rcfg_complete(self, request);
}

static inline
int scheduler_slot_complete(struct scheduler *self, struct accel_req *request)
{
    assert(self);

    return self->slot_complete(self, request);
}

static inline
int scheduler_slot_timeout(struct scheduler *self, struct accel_req *request)
{
    assert(self);

    return self->slot_timeout(self, request);
}

static inline
void scheduler_free(struct scheduler *self)
{
    if (self)
        self->free(self);
}

//---------------------------------------------------------------------------------------------

#endif /* SCHEDULER_H_ */
