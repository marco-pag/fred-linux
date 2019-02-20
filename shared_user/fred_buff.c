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

#include <assert.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "fred_buff.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

void fred_buff_init(struct fred_user_buff *buff)
{
	assert(buff);

	buff->map_addr = NULL;
	buff->file_d = 0;
	buff->length = 0;
}

void* fred_buff_map(struct fred_user_buff *buff)
{
	assert(buff);

	buff->file_d = open(buff->dev_name, O_RDWR);
	if (buff->file_d < 1) {
		ERROR_PRINT("buff: unable to open fred buffer file descriptor: %s\n", buff->dev_name);
		return NULL;
	}

	if (buff->map_addr) {
		DBG_PRINT("buff: waring! buffer is already mapped!\n");
		return NULL;
	}

	/* Map the whole buffer into the process user space */
	buff->map_addr = mmap(NULL, buff->length, PROT_READ | PROT_WRITE, MAP_SHARED, buff->file_d, 0);
	if (buff->map_addr == MAP_FAILED) {
		ERROR_PRINT("buff: failed to mmap buffer\n");
		return NULL;
	}


	return buff->map_addr;
}

void fred_buff_unmap(struct fred_user_buff *buff)
{
	assert(buff);

	if (!buff->map_addr) {
		DBG_PRINT("buff: waring! buffer is not mapped!\n");
		return;
	}

	if (munmap(buff->map_addr, buff->length)) {
		ERROR_PRINT("buff: could not unmap buff: %p, length:%u \n", buff->map_addr, buff->length);
		return;
	}

	close(buff->file_d);
}
