/*
 * Fred for Linux. Experimental support.
 *
 * Copyright (C) 2018, Marco Pagani, ReTiS Lab.
 * <marco.pag(at)outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
*/

#include <stdio.h>
#include <assert.h>

#include "decoup_drv.h"

#define REG_WRITE(BaseAddress, RegOffset, Data) \
    *(volatile unsigned int *)((BaseAddress) + (RegOffset)) = (unsigned int)(Data)

#define REG_READ(BaseAddress, RegOffset) \
    *(volatile unsigned int *)((BaseAddress) + (RegOffset))

void decoup_drv_decouple(uio_dev_ft *uio_dev)
{
	uint32_t base_addr;

	assert(uio_dev);

	base_addr = uio_get_base_addr(uio_dev);

	// Enable slot decoupler (decouple reconfigurable slot)
	REG_WRITE(base_addr, 0, 1);

	assert(REG_READ(base_addr, 0) == 1);
}

void decoup_drv_couple(uio_dev_ft *uio_dev)
{
	uint32_t base_addr;

	assert(uio_dev);

	base_addr = uio_get_base_addr(uio_dev);

	// Disable slot decoupler
	REG_WRITE(base_addr, 0, 0);

	assert(REG_READ(base_addr, 0) == 0);
}
