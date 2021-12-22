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

#ifndef DECOUP_DRV_XIL_H_
#define DECOUP_DRV_XIL_H_

#include "decoup_drv.h"

//---------------------------------------------------------------------------------------------

struct decoup_drv_xil {
    // ------------------------//
    struct decoup_drv decoup_drv;
    // ------------------------//

    // Driver UIO component
    struct uio_dev *uio_dev;
};

//---------------------------------------------------------------------------------------------

int decoup_drv_xil_init(struct decoup_drv **self, const char *dev_name);

//---------------------------------------------------------------------------------------------


#endif /* DECOUP_DRV_XIL_H_ */
