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

#ifndef FRED_SYS_H_
#define FRED_SYS_H_

#include "scheduler.h"
#include "sys_layout.h"
#include "reactor.h"
#include "sw_tasks_listener.h"
#include "devcfg.h"

#include "../srv_support/buffctl.h"

//---------------------------------------------------------------------------------------------

struct fred_sys;

//---------------------------------------------------------------------------------------------

int fred_sys_init(struct fred_sys **self, const char *arch_file,
				  const char *hw_tasks_file);

void fred_sys_free(struct fred_sys *self);

int fred_sys_run(struct fred_sys *self);

//---------------------------------------------------------------------------------------------

#endif /* FRED_SYS_H_ */
