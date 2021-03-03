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

#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "sched_mode.h"
#include "../utils/dbg_print.h"

int sched_mode_set_fp(int priority)
{
	int priority_min;
	int priority_max;
	struct sched_param param;

	priority_min = sched_get_priority_min(SCHED_POLICY);
	priority_max = sched_get_priority_max(SCHED_POLICY);

	param.sched_priority = priority_max - priority;
	assert(param.sched_priority >= priority_min);
	assert(param.sched_priority <= priority_max);

	if (sched_setscheduler(0, SCHED_POLICY, &param)) {
		ERROR_PRINT("Error while setting scheduler: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}
