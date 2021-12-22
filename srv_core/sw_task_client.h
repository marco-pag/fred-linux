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

#ifndef SW_TASK_CLIENT_H_
#define SW_TASK_CLIENT_H_

#include "../parameters.h"
#include "accel_req.h"
#include "sys_layout.h"
#include "hw_task.h"
#include "../srv_support/buffctl.h"
#include "scheduler.h"

//---------------------------------------------------------------------------------------------

struct sw_task_client {
    // ------------------------//
    struct event_handler handler;               // Handler interface
    // ------------------------//

    int conn_sock;                              // Handle

    enum sw_task_client_state {
        CLIENT_EMPTY,
        CLIENT_READY,
        CLIENT_BUSY
    } state;

    struct hw_task *hw_tasks[MAX_HW_TASKS];     // Associated hw-tasks
    int hw_tasks_count;

    // And data buffers (one set for each hw-task)
    struct fred_buff_if *data_buffs_ifs[MAX_HW_TASKS][MAX_DATA_BUFFS];

    struct scheduler *scheduler;                // Scheduler state machine
    struct sys_layout *sys;                     // System layout

    buffctl_ft *buffctl;                        // To allocate buffers (not owning)

    struct accel_req accel_req;                 // Max *one* request at time no
                                                // need to allocate dynamically
};

//---------------------------------------------------------------------------------------------

int sw_task_client_init(struct event_handler **self, int list_sock, struct sys_layout *sys,
                        struct scheduler *scheduler, buffctl_ft *buffctl);

//---------------------------------------------------------------------------------------------

#endif /* SW_TASK_CLIENT_H_ */
