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

#ifndef SLOT_DRV_H_
#define SLOT_DRV_H_

#include <stdint.h>

#include "uio_drv.h"

//---------------------------------------------------------------------------------------------

// Match hardware type
typedef uint32_t args_t;

//---------------------------------------------------------------------------------------------

#define slot_drv_dev_init(uio_dev, dev_name) \
		uio_dev_init(uio_dev, dev_name)

#define	slot_drv_dev_free(uio_dev) \
		uio_dev_free(uio_dev)

#define slot_drv_read_for_irq(uio_dev) \
		uio_read_for_irq(uio_dev)

#define	slot_drv_clear_gic(uio_dev) \
		uio_clear_gic(uio_dev)

#define slot_drv_get_fd(uio_dev) \
		uio_get_fd(uio_dev)

//---------------------------------------------------------------------------------------------

void slot_drv_enable_irq(uio_dev_ft *uio_dev);

void slot_drv_clear_int(uio_dev_ft *uio_dev);

uint32_t slot_drv_get_id(const uio_dev_ft *uio_dev);

int slot_drv_start_compute(uio_dev_ft *uio_dev, const args_t *args, int args_size);

void slot_drv_wait_for_compl(const uio_dev_ft *uio_dev);

//---------------------------------------------------------------------------------------------

#endif /* SLOT_DRV_H_ */
