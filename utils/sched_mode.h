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

#ifndef SCHED_MODE_H_
#define SCHED_MODE_H_

#define SCHED_POLICY SCHED_FIFO

// Set the priority of the calling thread.
// Possible values are form 0 to 99.
// 0 corresponds to the max priority
int sched_mode_set_fp(int priority);

#endif
