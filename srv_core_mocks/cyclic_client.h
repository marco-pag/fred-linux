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

#ifndef CYCLIC_CLIENT_H_
#define CYCLIC_CLIENT_H_

#include "../parameters.h"
#include "../srv_core/accel_req.h"
#include "../srv_core/sys_layout.h"
#include "../srv_core/scheduler.h"
#include "../srv_core/hw_task.h"

//---------------------------------------------------------------------------------------------

struct cyclic_sw_tasks_client {
    // ------------------------//
    struct event_handler handler;               // Handler (inherits)
    // ------------------------//

    int out_fd;                                 // Handle (source)
    int in_fd;

    struct hw_task *hw_tasks[MAX_HW_TASKS];     // Associated hw-tasks
    int hw_tasks_count;

    int next_hw_task;

    // And data buffers (one set for each hw-task)
    struct fred_buff_if *data_buffs_ifs[MAX_HW_TASKS][MAX_DATA_BUFFS];

    struct scheduler *scheduler;                // Scheduler state machine
    struct sys_layout *sys;                     // System layout

    buffctl_ft *buffctl;                        // To allocate buffers (not owning)

    struct accel_req accel_req;                 // Max *one* request at time no
                                                // need to allocate dynamically
};

//---------------------------------------------------------------------------------------------

int cyclic_sw_tasks_client_init(struct event_handler **self, struct sys_layout *sys,
                                struct scheduler *scheduler, buffctl_ft *buffctl);

//---------------------------------------------------------------------------------------------

#endif /* CYCLIC_CLIENT_H_ */
