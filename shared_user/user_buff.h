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

#ifndef USER_BUFF_H_
#define USER_BUFF_H_

#include <stddef.h>
#include "../parameters.h"

//---------------------------------------------------------------------------------------------

struct user_buff {
    void *map_addr;
    int file_d;
    size_t length;
    char dev_name[MAX_PATH];
};

//---------------------------------------------------------------------------------------------

void user_buff_init(struct user_buff *buff);

void *user_buff_map(struct user_buff *buff);

void user_buff_unmap(struct user_buff *buff);

size_t user_buff_get_size(const struct user_buff *buff);

//---------------------------------------------------------------------------------------------

#endif /* USER_H_ */
