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

#ifndef UIO_DRV_H_
#define UIO_DRV_H_

#include <stdint.h>

#include "../parameters.h"

//---------------------------------------------------------------------------------------------

// Simple -single map- UIO device
struct uio_dev {
    // Device tree name and UIO number
    // UIO name: (uio<num>)
    uint32_t uio_num;
    char dt_name[MAX_NAMES];

    // Device registers
    uintptr_t regs_addr;
    size_t regs_size;

    // UIO device fd (when opened)
    int uio_fd;
    // Base address when mapped into process space
    uintptr_t map_base;
};

//---------------------------------------------------------------------------------------------

int uio_dev_init(struct uio_dev **uio_dev, const char* dev_name);

void uio_dev_free(struct uio_dev *uio_dev);

int uio_get_fd(const struct uio_dev *uio_dev);

uintptr_t uio_get_base_addr(const struct uio_dev *uio_dev);

int uio_read_for_irq(struct uio_dev *uio_dev);

void uio_clear_gic(struct uio_dev *uio_dev);

//---------------------------------------------------------------------------------------------

#endif /* UIO_DRV_H_ */
