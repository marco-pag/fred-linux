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

#include <stdio.h>

#include "srv_core/fred_sys.h"
#include "utils/sched_mode.h"

//---------------------------------------------------------------------------------------------

const char *ARCH_FILE = "arch.csv";
const char *HW_TASKS = "hw_tasks.csv";

//---------------------------------------------------------------------------------------------

int main ()
{
	int retval;
	struct fred_sys *fred_sys;

	sched_mode_set_fp(0);

	retval = fred_sys_init(&fred_sys, ARCH_FILE, HW_TASKS);
	if (retval < 0) {
		return -1;
	}

	// Event loop
	fred_sys_run(fred_sys);

	fred_sys_free(fred_sys);

	return 0;
}

