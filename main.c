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

#include <stdio.h>

#include "srv_core/fred_sys.h"
#include "utils/sched_mode.h"

//---------------------------------------------------------------------------------------------

static const char *ARCH_FILE = "arch.csv";
static const char *HW_TASKS_FILE = "hw_tasks.csv";

//---------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	int retval;
	struct fred_sys *fred_sys;
	enum fred_sys_mode mode;

	// To be removed
	sched_mode_set_fp(0);

	if (argc > 1)
		mode = FRED_RCFG_TEST_MODE;
	else
		mode = FRED_NORMAL_MODE;

	retval = fred_sys_init(&fred_sys, ARCH_FILE, HW_TASKS_FILE, mode);
	if (retval < 0)
		return -1;

	// Event loop
	fred_sys_run(fred_sys);

	fred_sys_free(fred_sys);

	return 0;
}

