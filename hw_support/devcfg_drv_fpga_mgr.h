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

#ifndef DEVCFG_DRV_FPGA_MGR_H_
#define DEVCFG_DRV_FPGA_MGR_H_

#include <stdint.h>

#include "devcfg_drv.h"
#include "../srv_core/phy_bit.h"

//---------------------------------------------------------------------------------------------

struct devcfg_drv_fpga_mgr {
    // ------------------------//
    struct devcfg_drv devcfg_drv;
    // ------------------------//

    int phy_bit_rcfg_done_fd;
    int phy_bit_rcfg_start_fd;
    int phy_bit_addr_fd;
    int phy_bit_size_fd;
};

//---------------------------------------------------------------------------------------------

int devcfg_drv_fpga_mgr_init(struct devcfg_drv **self);

//---------------------------------------------------------------------------------------------

#endif /* DEVCFG_DRV_FPGA_MGR_H_ */
