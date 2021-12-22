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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "decoup_drv_xil.h"
#include "uio_drv.h"

//---------------------------------------------------------------------------------------------

#define REG_WRITE(BaseAddress, RegOffset, Data) \
                *(volatile unsigned int *)((BaseAddress) + (RegOffset)) = (unsigned int)(Data)

#define REG_READ(BaseAddress, RegOffset) \
                *(volatile unsigned int *)((BaseAddress) + (RegOffset))

//---------------------------------------------------------------------------------------------

void decoup_drv_xil_decouple_(struct decoup_drv *self)
{
    struct decoup_drv_xil *xil_dev_drv;
    uintptr_t base_addr;

    assert(self);

    xil_dev_drv = (struct decoup_drv_xil *)self;

    base_addr = uio_get_base_addr(xil_dev_drv->uio_dev);

    // Enable slot decoupler (decouple reconfigurable slot)
    REG_WRITE(base_addr, 0, 1);

    assert(REG_READ(base_addr, 0) == 1);
}

void decoup_drv_xil_couple_(struct decoup_drv *self)
{
    struct decoup_drv_xil *xil_dev_drv;
    uintptr_t base_addr;

    assert(self);

    xil_dev_drv = (struct decoup_drv_xil *)self;

    base_addr = uio_get_base_addr(xil_dev_drv->uio_dev);

    // Disable slot decoupler
    REG_WRITE(base_addr, 0, 0);

    assert(REG_READ(base_addr, 0) == 0);
}

void decoup_drv_xil_free_(struct decoup_drv *self)
{
    struct decoup_drv_xil *xil_dev_drv;

    if(!self)
        return;

    xil_dev_drv = (struct decoup_drv_xil *)self;

    uio_dev_free(xil_dev_drv->uio_dev);

    free(xil_dev_drv);
}

//---------------------------------------------------------------------------------------------

int decoup_drv_xil_init(struct decoup_drv **self, const char *dev_name)
{
    struct decoup_drv_xil *xil_dev_drv;
    int retval;

    assert(self);

    *self = NULL;

    xil_dev_drv = calloc(1, sizeof(*xil_dev_drv));
    if (!xil_dev_drv)
        return -1;

    // Decoupler interface
    xil_dev_drv->decoup_drv.decouple = decoup_drv_xil_decouple_;
    xil_dev_drv->decoup_drv.couple = decoup_drv_xil_couple_;
    xil_dev_drv->decoup_drv.free = decoup_drv_xil_free_;

    // Initialize UIO component
    retval = uio_dev_init(&xil_dev_drv->uio_dev, dev_name);
    if (retval) {
        free(xil_dev_drv);
        return -1;
    }

    *self = &xil_dev_drv->decoup_drv;

    return 0;
}
