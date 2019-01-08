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

#ifndef FRED_BUFF_H_
#define FRED_BUFF_H_

#include <stddef.h>
#include "../parameters.h"

//---------------------------------------------------------------------------------------------

struct fred_user_buff {
	void *map_addr;
	int file_d;
	size_t length;
	char dev_name[MAX_PATH];
};

//---------------------------------------------------------------------------------------------

void fred_buff_init(struct fred_user_buff *buff);

void *fred_buff_map(struct fred_user_buff *buff);

void fred_buff_unmap(struct fred_user_buff *buff);

//---------------------------------------------------------------------------------------------

#endif /* BUFF_H_ */
