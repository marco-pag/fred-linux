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

#ifndef DECOUP_DRV_H_
#define DECOUP_DRV_H_

#include "uio_drv.h"

#define decoup_drv_dev_init(uio_dev, dev_name) \
		uio_dev_init(uio_dev, dev_name)

#define	decoup_drv_dev_free(uio_dev) \
		uio_dev_free(uio_dev)

void decoup_drv_decouple(uio_dev_ft *uio_dev);

void decoup_drv_couple(uio_dev_ft *uio_dev);

#endif /* DECOUP_DRV_H_ */
