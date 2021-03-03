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

#ifndef BUFFCTL_H_
#define BUFFCTL_H_

#include "../shared_kernel/fred_buffctl_shared.h"

//---------------------------------------------------------------------------------------------

typedef struct buffctl_ buffctl_ft;

//---------------------------------------------------------------------------------------------

int buffctl_open(buffctl_ft **buffctl, const char *dev_name);

int buffctl_close(buffctl_ft *buffctl);

int buffctl_alloc_buff(buffctl_ft *buffctl, struct fred_buff_if **buff_if, size_t size);

int buffctl_free_buff(buffctl_ft *buffctl, struct fred_buff_if *buff_if);

//---------------------------------------------------------------------------------------------

#endif /* BUFFCTL_H_ */
