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

#include <stdint.h>
#include "../srv_core/phy_bit.h"

//---------------------------------------------------------------------------------------------

typedef struct devcfg_drv_ devcfg_drv;

//---------------------------------------------------------------------------------------------

int devcfg_drv_init(devcfg_drv **self);

void devcfg_drv_free(devcfg_drv *devcfg);

int devcfg_drv_get_fd(const devcfg_drv *devcfg);

int devcfg_drv_start_prog(const devcfg_drv *devcfg, const struct phy_bit *phy_bit);

int64_t devcfg_drv_clear_evt(devcfg_drv *devcfg);

//---------------------------------------------------------------------------------------------

#endif /* DEVCFG_DRV_H_ */
