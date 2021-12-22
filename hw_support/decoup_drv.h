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

#ifndef DECOUP_DRV_H_
#define DECOUP_DRV_H_

#include <assert.h>

//---------------------------------------------------------------------------------------------

struct decoup_drv {
    void (*decouple)(struct decoup_drv *self);
    void (*couple)(struct decoup_drv *self);

    void (*free)(struct decoup_drv *self);
};


//---------------------------------------------------------------------------------------------

static inline
void decoup_drv_decouple(struct decoup_drv *self)
{
    assert(self);

    self->decouple(self);
}

static inline
void decoup_drv_couple(struct decoup_drv *self)
{
    assert(self);

    self->couple(self);
}

static inline
void decoup_drv_free(struct decoup_drv *self)
{
    assert(self);

    self->free(self);
}

//---------------------------------------------------------------------------------------------


#endif /* DECOUP_DRV_H_ */
