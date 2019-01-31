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

#ifndef UIO_DRV_H_
#define UIO_DRV_H_

#include <stdint.h>

//---------------------------------------------------------------------------------------------

typedef struct uio_dev_ uio_dev_ft;

//---------------------------------------------------------------------------------------------

int uio_dev_init(uio_dev_ft **uio_dev, const char* dev_name);

void uio_dev_free(uio_dev_ft *uio_dev);

int uio_get_fd(const uio_dev_ft *uio_dev);

uintptr_t uio_get_base_addr(const uio_dev_ft *uio_dev);

int uio_read_for_irq(uio_dev_ft *uio_dev);

void uio_clear_gic(uio_dev_ft *uio_dev);

//---------------------------------------------------------------------------------------------

#endif /* UIO_DRV_H_ */
