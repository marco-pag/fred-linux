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

#ifndef FRED_SYS_H_
#define FRED_SYS_H_


//---------------------------------------------------------------------------------------------

struct fred_sys;

enum fred_sys_mode {
	FRED_NORMAL_MODE,			// Regular operation
	FRED_RCFG_TEST_MODE,		// Reconfiguration test
	FRED_HW_TASKS_TEST_MODE		// Hw-tasks execution test (to be implemented)
};

//---------------------------------------------------------------------------------------------

int fred_sys_init(struct fred_sys **self, const char *arch_file,
				  const char *hw_tasks_file, enum fred_sys_mode mode);

void fred_sys_free(struct fred_sys *self);

void fred_sys_run(struct fred_sys *self);

//---------------------------------------------------------------------------------------------

#endif /* FRED_SYS_H_ */
