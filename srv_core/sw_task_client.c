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

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "sw_task_client.h"
#include "../utils/dbg_print.h"
#include "../shared_user/fred_msg.h"
#include "../shared_user/user_buff.h"
#include "../utils/fd_utils.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

static
int alloc_data_buff_(struct sw_task_client *self, int task_idx)
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
void free_all_data_buff_(struct sw_task_client *self)
{
    for (int i = 0; i < self->hw_tasks_count; ++i) {
        for (int j = 0; j < MAX_DATA_BUFFS; ++j) {
            if (self->data_buffs_ifs[i][j])
                buffctl_free_buff(self->buffctl, self->data_buffs_ifs[i][j]);
        }
    }
}

static inline
int write_to_client_(int socket, const void *data, unsigned int data_len)
{
    int retval;

    retval = write(socket, data, data_len);
    if (retval != data_len) {
        ERROR_PRINT("fred_sys: unable to reach client. Error: %s\n", strerror(errno));
        return 1;
    }

    return 0;
}

static inline
int send_fred_message_(int socket, int head, uint32_t arg)
{
    struct fred_msg msg;

    msg.head = head;
    msg.arg = arg;

    return write_to_client_(socket, &msg, sizeof(msg));
}

static
int send_user_data_buffs_(struct sw_task_client *self, int task_idx)
{
    int retval;
    int data_buffs_count;
    const unsigned int *data_buffs_sizes;
    char usr_dev_name[MAX_PATH];

    // User buffer representations (one at time)
    struct user_buff user_buffs[MAX_DATA_BUFFS];

    // Get hw-task properties
    data_buffs_count = hw_task_get_data_buffs_count(self->hw_tasks[task_idx]);
    data_buffs_sizes = hw_task_get_data_buffs_sizes(self->hw_tasks[task_idx]);

    // Build hw-tasks' data buffers user representations
    for (int i = 0; i < data_buffs_count; ++i) {
        // Set buffer length
        user_buffs[i].length = data_buffs_sizes[i];

        // Convert device name (from kernel mod) into user form
        // es: "fred!buffN" -> "/dev/fred/buffN"
        snprintf(usr_dev_name, MAX_PATH, "/dev/%s",
                    self->data_buffs_ifs[task_idx][i]->dev_name);
        usr_dev_name[strcspn(usr_dev_name, "!")] = '/';

        strncpy(user_buffs[i].dev_name, usr_dev_name,
                sizeof(user_buffs[i].dev_name) -1);
    }

    // Send to the client the number of data buffers
    retval = send_fred_message_(self->conn_sock, FRED_MSG_BUFFS, data_buffs_count);
    if (retval)
        return 1;

    // Send to the client the buffers representations
    retval = write_to_client_(self->conn_sock, user_buffs,
                                sizeof(user_buffs[0]) * data_buffs_count);
    if (retval)
        return 1;

    return 0;
}

// Return values:
//  1) communication or allocation error (single sw-task issue)
// -1) system error
static inline
int process_msg_(struct sw_task_client *self, const struct fred_msg *msg)
{
    uint32_t arg;
    int idx;
    int retval;
    struct hw_task *hw_task = NULL;
    int data_buffs_count;

    switch (fred_msg_get_head(msg)) {
    case FRED_MSG_INIT:
        if (self->state != CLIENT_EMPTY) {
            retval = send_fred_message_(self->conn_sock, FRED_MSG_ERROR, 0);
        } else {
            self->state = CLIENT_READY;
            retval = send_fred_message_(self->conn_sock, FRED_MSG_ACK, 0);
        }
        break;

    case FRED_MSG_BIND:
        if (self->state != CLIENT_READY) {
            retval = send_fred_message_(self->conn_sock, FRED_MSG_ERROR, 0);
        } else {
            // Get hw-task id from request
            arg = fred_msg_get_arg(msg);
            hw_task = sys_layout_get_hw_task(self->sys, arg);
            if (!hw_task) {
                ERROR_PRINT("fred_sys: unable to find hw-task id: %u\n", arg);
                retval = send_fred_message_(self->conn_sock, FRED_MSG_ERROR, 0);
                break;

            } else {
                // If the hw-task has been already disable due to an overrun
                if (hw_task_get_banned(hw_task)) {
                    retval = send_fred_message_(self->conn_sock, FRED_MSG_ERROR, 0);
                    break;
                }

                // Cannot bind any additional hw-task
                if (self->hw_tasks_count >= MAX_HW_TASKS - 1) {
                    ERROR_PRINT("fred_sys: critical: maximum number of hw-tasks"
                                " exceeded: detaching client\n");
                    retval = send_fred_message_(self->conn_sock, FRED_MSG_ERROR, 0);
                    break;
                }

                // Add hw-task reference to client's list
                // "hw_tasks_count" index in the first free slot
                self->hw_tasks[self->hw_tasks_count] = hw_task;

                // Allocate hw-tasks data buffers (pass current index)
                retval = alloc_data_buff_(self, self->hw_tasks_count);
                if (retval) {
                    ERROR_PRINT("fred_sys: critical: could not allocate data buffer,"
                                "detaching client\n");
                    break;
                }

                // Send buffer to the client
                retval = send_user_data_buffs_(self, self->hw_tasks_count);
                if (retval) {
                    ERROR_PRINT("fred_sys: critical: communication error while"
                                "binding data buffers: detaching client\n");
                    break;
                }

                // Update hw-tasks count
                self->hw_tasks_count++;
            }
        }
        break;

    case FRED_MSG_RUN:
        if (self->state != CLIENT_READY) {
            retval = send_fred_message_(self->conn_sock, FRED_MSG_ERROR, 0);
        } else {
            // Get hw-task id from request
            arg = fred_msg_get_arg(msg);
            idx = -1;
            // Find requested hw-task
            for (int i = 0; i < self->hw_tasks_count; ++i) {
                if (hw_task_get_id(self->hw_tasks[i]) == arg) {
                    idx = i;
                    break;
                }
            }

            // If the requested hw-task is not associated with this sw-task
            if (idx < 0) {
                retval = send_fred_message_(self->conn_sock, FRED_MSG_ERROR, 0);
                break;
            }

            // If the hw-task exist but it has been already disable due to an overrun
            if (hw_task_get_banned(self->hw_tasks[idx])) {
                retval = send_fred_message_(self->conn_sock, FRED_MSG_ERROR, 0);
                break;
            }

            // The hw-task exist: build and fire acceleration request
            accel_req_unbind(&self->accel_req);
            accel_req_set_hw_task(&self->accel_req, self->hw_tasks[idx]);
            // Set hardware arguments (memory buffer pointers)
            data_buffs_count = hw_task_get_data_buffs_count(self->hw_tasks[idx]);
            accel_req_set_args_size(&self->accel_req, data_buffs_count);
            for (int j = 0; j < data_buffs_count; ++j) {
                accel_req_set_args(&self->accel_req, j,
                    fred_buff_if_get_phy_addr(self->data_buffs_ifs[idx][j]));
            }
            // Pass acceleration request to the scheduler
            retval = scheduler_push_accel_req(self->scheduler, &self->accel_req);
        }
        break;

    default:
        retval = send_fred_message_(self->conn_sock, FRED_MSG_ERROR, 0);
        break;
    }

    // If unable to reach client process
    if (retval > 0) {
        DBG_PRINT("fred_sys: client error: detaching client\n");
        return 1;

    // Critical error, notify all clients and shutdown the server
    } else if (retval < 0) {
        ERROR_PRINT("fred_sys: critical error while processing client request\n");
        return -1;

    } else {
        return 0;
    }
}

//---------------------------------------------------------------------------------------------

static
int sw_task_client_notify_action_(void *notifier, enum notify_action_msg msg)
{
    struct sw_task_client *self;
    int retval;

    assert(notifier);

    self = (struct sw_task_client *)notifier;

    switch (msg) {
        case NOTIFY_ACTION_DONE:
            // Notify the client that his acceleration request has been completed
            retval = send_fred_message_(self->conn_sock, FRED_MSG_DONE, 0);
            break;
        case NOTIFY_ACTION_OVERRUN:
        default:
            // Notify the client that the hw-task overrun and will be disabled
            retval = send_fred_message_(self->conn_sock, FRED_MSG_OVERRUN, 0);
            break;
    }

    return retval;
}

// ---------------------- Functions to implement event_handler interface ----------------------

static
int get_fd_handle_(const struct event_handler *self)
{
    struct sw_task_client *cp;

    assert(self);

    cp = (struct sw_task_client *)self;
    return cp->conn_sock;
}

static
void get_name_(const struct event_handler *self, char *msg, int msg_size)
{
    struct sw_task_client *cp;

    assert(self);
    assert(msg);

    cp = (struct sw_task_client *)self;
    snprintf(msg, msg_size, "sw-task client on fd: %d", cp->conn_sock);
}

static
int handle_event_(struct event_handler *self)
{
    struct sw_task_client *cp;

    struct fred_msg msg;
    ssize_t nread;

    assert(self);

    cp = (struct sw_task_client *)self;
    nread = read(cp->conn_sock, &msg, sizeof(msg));
    if (nread < 0) {
        ERROR_PRINT("fred_sys: error reading client message from socket: %s\n",
                    strerror(errno));
        return -1;

    } else if (nread == 0) {
        // Client has closed the connection
        DBG_PRINT("fred_sys: client disconnected\n");
        return 1;
    }

    return process_msg_(cp, &msg);
}

static
void free_(struct event_handler *self)
{
    struct sw_task_client *cp;

    assert(self);

    cp = (struct sw_task_client *)self;

    free_all_data_buff_(cp);

    close(cp->conn_sock);
    free(cp);
}

//---------------------------------------------------------------------------------------------


int sw_task_client_init(struct event_handler **self, int list_sock, struct sys_layout *sys,
                        struct scheduler *scheduler, buffctl_ft *buffctl)
{
    struct sw_task_client *client;
    int retval;
    struct sockaddr_un cli_addr;
    socklen_t cli_addr_len;

    assert(sys);
    assert(scheduler);
    assert(buffctl);

    *self = NULL;

    // Allocate and set everything to 0
    client = calloc(1, sizeof (*client));
    if (!client)
        return -1;

    event_handler_assign_id(&client->handler);

    // Accept connection, the event handler will be registered
    // to the reactor by the server
    cli_addr_len = sizeof(cli_addr);
    client->conn_sock = accept(list_sock, (struct sockaddr *)&cli_addr, &cli_addr_len);
    if (client->conn_sock < 0) {
        ERROR_PRINT("fred_sys: error on connect: %s\n", strerror(errno));
        free(client);
        return -1;
    }

    retval = fd_utils_set_fd_nonblock(client->conn_sock);
    if (retval) {
        free(client);
        return -1;
    }

    // Set properties and methods
    client->sys = sys;
    client->scheduler = scheduler;
    client->buffctl = buffctl;
    client->state = CLIENT_EMPTY;

    // Event handler interface
    client->handler.handle_event = handle_event_;
    client->handler.get_fd_handle = get_fd_handle_;
    client->handler.get_name = get_name_;
    client->handler.free = free_;

    // Link notify action
    accel_req_set_notifier(&client->accel_req, sw_task_client_notify_action_, client);

    *self = &client->handler;

    return 0;
}
