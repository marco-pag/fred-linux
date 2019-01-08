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

#ifndef DEVCFG_DRV_H_
#define DEVCFG_DRV_H_

#include <stddef.h>
#include "../shared_kernel/fred_xdevcfg_mod_shared.h"

typedef struct devcfg_drv_ devcfg_drv;

int devcfg_drv_init(devcfg_drv **self);

void devcfg_drv_free(devcfg_drv *devcfg);

int devcfg_drv_get_fd(const devcfg_drv *devcfg);

int devcfg_drv_start_prog(const devcfg_drv *devcfg,
							const struct phy_bitstream *phy_bit);

uint32_t devcfg_drv_clear_evt(const devcfg_drv *devcfg);

int devcfg_drv_write_legacy(const devcfg_drv *devcfg, void *bit, size_t bit_len);

#endif /* DEVCFG_DRV_H_ */
