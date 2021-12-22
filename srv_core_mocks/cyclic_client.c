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

#include "../srv_core_mocks/cyclic_client.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "../shared_user/fred_msg.h"
#include "../shared_user/user_buff.h"
#include "../utils/fd_utils.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

static
int alloc_data_buff_(struct cyclic_sw_tasks_client *self, int task_idx)
{
    int retval;
    const unsigned int *data_buffs_sizes;
    int data_buffs_count;

    data_buffs_count = hw_task_get_data_buffs_count(self->hw_tasks[task_idx]);
    data_buffs_sizes = hw_task_get_data_buffs_sizes(self->hw_tasks[task_idx]);

    // Allocate all data buffers for that hw-task
    for (int i = 0; i < data_buffs_count; ++i) {
        retval = buffctl_alloc_buff(self->buffctl,
                                    &(self->data_buffs_ifs[task_idx][i]),
                                    data_buffs_sizes[i]);

        if (retval)
            return 1;
    }

    return 0;
}

static
void free_all_data_buff_(struct cyclic_sw_tasks_client *self)
{
    for (int i = 0; i < self->hw_tasks_count; ++i) {
        for (int j = 0; j < MAX_DATA_BUFFS; ++j) {
            if (self->data_buffs_ifs[i][j])
                buffctl_free_buff(self->buffctl, self->data_buffs_ifs[i][j]);
        }
    }
}

static
int bind_all_hw_tasks_(struct cyclic_sw_tasks_client *self)
{
    int retval = 0;

    // Bind with all hw-tasks available on the system
    self->hw_tasks_count = sys_layout_get_hw_tasks(self->sys, self->hw_tasks);
    if (self->hw_tasks_count <= 0) {
        ERROR_PRINT("fred_sys: cyclic sw-tasks client: no hw-tasks on sys\n");
        return -1;
    }

    for (int i = 0; i < self->hw_tasks_count; ++i) {
        // Allocate hw-tasks data buffers
        retval = alloc_data_buff_(self, i);
        if (retval) {
            ERROR_PRINT("fred_sys: cyclic sw-tasks client: could not allocate data buffer\n");
            break;
        }

        DBG_PRINT("fred_sys: cyclic sw-tasks client: bind to hw-tasks %s\n",
                    hw_task_get_name(self->hw_tasks[i]));
    }

    return retval;
}

static
void build_accel_request_(struct cyclic_sw_tasks_client *self, int hw_task_idx)
{
    int data_buffs_count;

    // Build and fire acceleration request
    accel_req_unbind(&self->accel_req);
    accel_req_set_hw_task(&self->accel_req, self->hw_tasks[hw_task_idx]);
    // Set hardware arguments (memory buffer pointers)
    data_buffs_count = hw_task_get_data_buffs_count(self->hw_tasks[hw_task_idx]);
    accel_req_set_args_size(&self->accel_req, data_buffs_count);
    for (int j = 0; j < data_buffs_count; ++j) {
        accel_req_set_args(&self->accel_req, j,
            fred_buff_if_get_phy_addr(self->data_buffs_ifs[hw_task_idx][j]));
    }
}

//---------------------------------------------------------------------------------------------

static
int cyclic_client_notify_action_(void *notifier, enum notify_action_msg msg)
{
    int retval;
    struct cyclic_sw_tasks_client *cp;

    assert(notifier);

    cp = (struct cyclic_sw_tasks_client *)notifier;

    // Trigger the next acceleration request event
    retval = fd_utils_byte_write(cp->in_fd);
    if (retval) {
        ERROR_PRINT("fred_sys: cyclic sw-tasks client: temporary file write error\n");
        return -1;
    }

    return 0;
}

// ---------------------- Functions to implement event_handler interface ----------------------

static
int get_fd_handle_(const struct event_handler *self)
{
    struct cyclic_sw_tasks_client *cp;

    assert(self);

    cp = (struct cyclic_sw_tasks_client *)self;
    return cp->out_fd;
}

static
void get_name_(const struct event_handler *self, char *msg, int msg_size)
{
    assert(self);
    assert(msg);

    struct cyclic_sw_tasks_client *cp;

    assert(self);

    cp = (struct cyclic_sw_tasks_client *)self;

    snprintf(msg, msg_size, "cyclic sw-tasks test client on fd: %d", cp->out_fd);
}

static
void free_(struct event_handler *self)
{
    struct cyclic_sw_tasks_client *cp;

    assert(self);

    cp = (struct cyclic_sw_tasks_client *)self;

    close(cp->in_fd);
    close(cp->out_fd);

    free_all_data_buff_(cp);

    free(cp);
}

static
int handle_event_(struct event_handler *self)
{
    int retval;
    struct cyclic_sw_tasks_client *cp;

    assert(self);

    cp = (struct cyclic_sw_tasks_client *)self;

    // Consume the event
    retval = fd_utils_byte_read(cp->out_fd);
    if (retval) {
        ERROR_PRINT("fred_sys: cyclic sw-tasks client: temporary file read error\n");
        return -1;
    }

    // Build the acceleration request for the next task (even if banned)
    // and pass it to the scheduler
    build_accel_request_(cp, cp->next_hw_task);
    retval = scheduler_push_accel_req(cp->scheduler, &cp->accel_req);
    if (retval)
        return -1;

    // Increment next hw-tasks cyclic counter
    cp->next_hw_task = (cp->next_hw_task + 1) % cp->hw_tasks_count;

    // The event has been served correctly
    return 0;
}

//---------------------------------------------------------------------------------------------

int cyclic_sw_tasks_client_init(struct event_handler **self, struct sys_layout *sys,
                                struct scheduler *scheduler, buffctl_ft *buffctl)
{
    struct cyclic_sw_tasks_client *client;
    int retval;

    assert(sys);
    assert(scheduler);
    assert(buffctl);

    *self = NULL;

    // Allocate and set everything to 0
    client = calloc(1, sizeof (*client));
    if (!client)
        return -1;

    event_handler_assign_id(&client->handler);

    // Set properties and methods
    client->scheduler = scheduler;
    client->sys = sys;
    client->buffctl = buffctl;

    // Event handler interface
    client->handler.handle_event = handle_event_;
    client->handler.get_fd_handle = get_fd_handle_;
    client->handler.get_name = get_name_;
    client->handler.free = free_;

    // Link notify action
    accel_req_set_notifier(&client->accel_req, cyclic_client_notify_action_, client);

    // Bind all available hw-tasks
    retval = bind_all_hw_tasks_(client);
    if (retval) {
        free(client);
        return -1;
    }

    retval = fd_utils_create_socket_pair(&client->in_fd, &client->out_fd);
    if (retval) {
        ERROR_PRINT("fred_sys: cyclic sw-tasks client: cannot create socket pair\n");
        free(client);
        return -1;
    }

    retval = fd_utils_set_fd_nonblock(client->out_fd);
    if (retval) {
        ERROR_PRINT("fred_sys: cyclic sw-tasks client: error on set non-block\n");
        free(client);
        return -1;
    }

    // Trigger the first acceleration request event
    retval = fd_utils_byte_write(client->in_fd);
    if (retval) {
        ERROR_PRINT("fred_sys: cyclic sw-tasks client: temporary file write error\n");
        free(client);
        return -1;
    }

    *self = &client->handler;

    return 0;
}
