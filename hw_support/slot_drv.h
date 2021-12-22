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

#ifndef SLOT_DRV_H_
#define SLOT_DRV_H_

#include <stdint.h>
#include <assert.h>

//---------------------------------------------------------------------------------------------

struct slot_drv {
    uint32_t (*get_id)(const struct slot_drv *self);
    int (*get_fd)(const struct slot_drv *self);
    
    void (*before_rcfg)(struct slot_drv *self);
    void (*after_rcfg)(struct slot_drv *self);
    
    int (*start_compute)(struct slot_drv *self, const uintptr_t *args, int args_size);
    void (*after_compute)(struct slot_drv *self);

    // Only for testing
    void (*wait_for_compl)(const struct slot_drv *self);

    void (*free)(struct slot_drv *self);
};


//---------------------------------------------------------------------------------------------

static inline
uint32_t slot_drv_get_id(const struct slot_drv *self)
{
    assert(self);

    return self->get_id(self);
}

static inline
int slot_drv_get_fd(const struct slot_drv *self)
{
    assert(self);

    return self->get_fd(self);
}

static inline
void slot_drv_before_rcfg(struct slot_drv *self)
{
    assert(self);

    self->before_rcfg(self);
}

static inline
void slot_drv_after_rcfg(struct slot_drv *self)
{
    assert(self);

    self->after_rcfg(self);
}

static inline
int slot_drv_start_compute(struct slot_drv *self, const uintptr_t *args, int args_size)
{
    assert(self);
    assert(args);
    assert(args_size >= 0);

    return self->start_compute(self, args, args_size);
}

static inline
void slot_drv_after_compute(struct slot_drv *self)
{
    assert(self);

    self->after_compute(self);
}

static inline
void slot_drv_wait_for_compl(const struct slot_drv *self)
{
    assert(self);

    self->wait_for_compl(self);
}

static inline
void slot_drv_free(struct slot_drv *self)
{
    assert(self);

    self->free(self);
}

//---------------------------------------------------------------------------------------------

#endif /* SLOT_DRV_H_ */
