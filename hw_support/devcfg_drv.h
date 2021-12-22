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

#ifndef DEVCFG_DRV_H_
#define DEVCFG_DRV_H_

#include <stdint.h>
#include <assert.h>
#include "../srv_core/phy_bit.h"

//---------------------------------------------------------------------------------------------

struct devcfg_drv {
    int (*get_fd)(struct devcfg_drv *self);

    int (*start_rcfg)(struct devcfg_drv *self, const struct phy_bit *phy_bit);

    // Should return reconfiguration time
    uint64_t (*after_rcfg)(struct devcfg_drv *self);

    void (*free)(struct devcfg_drv *self);
};

//---------------------------------------------------------------------------------------------

static inline
int devcfg_drv_get_fd(struct devcfg_drv *self)
{
    assert(self);

    return self->get_fd(self);
}

static inline
int devcfg_drv_start_rcfg(struct devcfg_drv *self, const struct phy_bit *phy_bit)
{
    assert(self);

    return self->start_rcfg(self, phy_bit);
}

static inline
uint64_t devcfg_drv_after_rcfg(struct devcfg_drv *self)
{
    assert(self);

    return self->after_rcfg(self);
}

static inline
void devcfg_drv_free(struct devcfg_drv *self)
{
    assert(self);

    self->free(self);
}

//---------------------------------------------------------------------------------------------

#endif /* DEVCFG_DRV_H_ */
