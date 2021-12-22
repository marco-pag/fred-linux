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

#ifndef SLOT_DRV_MASTER_H_
#define SLOT_DRV_MASTER_H_

#include "slot_drv.h"
#include "uio_drv.h"

//---------------------------------------------------------------------------------------------

struct slot_drv_master {
    // ------------------------//
    struct slot_drv slot_drv;
    // ------------------------//

    // Driver UIO component
    struct uio_dev *uio_dev;
};

//---------------------------------------------------------------------------------------------

int slot_drv_master_init(struct slot_drv **self, const char *dev_name);

//---------------------------------------------------------------------------------------------

#endif /* SLOT_DRV_MASTER_H_ */
