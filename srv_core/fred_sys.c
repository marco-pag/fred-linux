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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "sys_layout.h"
#include "reactor.h"
#include "sw_tasks_listener.h"
#include "devcfg.h"
#include "scheduler_fred.h"
#include "../srv_core_mocks/scheduler_fred_rand.h"
#include "signals_recv.h"
#include "../srv_support/buffctl.h"
#include "../utils/logger.h"
#include "../utils/dbg_print.h"
#include "../srv_core_mocks/cyclic_client.h"
#include "../hw_support/sys_hw_config.h"

#include "fred_sys.h"

//---------------------------------------------------------------------------------------------

static const char fred_logo[] =
"\n"
"_|_|_|_|  _|_|_|    _|_|_|_|  _|_|_|    \n"
"_|        _|    _|  _|        _|    _|  \n"
"_|_|_|    _|_|_|    _|_|_|    _|    _|  \n"
"_|        _|    _|  _|        _|    _|  \n"
"_|        _|    _|  _|_|_|_|  _|_|_|    \n"
"\n";

//---------------------------------------------------------------------------------------------

// Don't include handlers objects. Handlers will be deallocated
// by the reactor using the event_handler interface
struct fred_sys {
    // Configuration for hw components
    struct sys_hw_config hw_config;

    // Epoll event reactor
    struct reactor *reactor;

    // System layout: partition, slots, and hw-tasks
    struct sys_layout *layout;

    // Buffer allocator kernel module
    buffctl_ft *buffctl;

    // Reconfiguration device
    struct devcfg *devcfg;

    // FRED scheduler component (not a handler)
    struct scheduler *scheduler;
};

//---------------------------------------------------------------------------------------------

static
int init_base_sys_(struct fred_sys *self, const char *arch_file, const char *hw_tasks_file)
{
    int retval;

    // Initialize logger
    retval = logger_init();
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing logger\n");
        goto error_logger;
    }

    // Initialize event demultiplexer
    retval = reactor_init(&self->reactor);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing event reactor\n");
        goto error_reactor;
    }

    // Open kernel module interface file
    retval = buffctl_open(&self->buffctl, NULL);
    if (retval) {
        ERROR_PRINT("fred_sys: unable to open buffctl device\n");
        goto error_buffctl;
    }

    // Initialize partitions, slots, and hw-tasks
    retval = sys_layout_init(&self->layout, &self->hw_config, arch_file, hw_tasks_file,
                            self->scheduler, self->buffctl);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing system layout\n");
        goto error_sys_layout;
    }

    // For debugging purposes
    sys_layout_print(self->layout);

    retval = 0;
    goto out;

// Error handling chain
error_sys_layout:
    buffctl_close(self->buffctl);
error_buffctl:
    reactor_free(self->reactor);
error_reactor:
    logger_free();
error_logger:
    retval = -1;
out:
    return retval;
}

static
void free_base_sys_(struct fred_sys *self)
{
    if (!self)
        return;

    if (self->layout)
        sys_layout_free(self->layout);

    if (self->buffctl)
        buffctl_close(self->buffctl);

    if (self->reactor)
        reactor_free(self->reactor);

    logger_free();
}

//---------------------------------------------------------------------------------------------

int init_normal_mode_(struct fred_sys *self, const char *arch_file, const char *hw_tasks_file)
{
    struct event_handler *sw_tasks_listener;
    struct event_handler *signals_receiver;
    int retval;

    DBG_PRINT("fred_sys: starting in normal mode\n");

    sys_hw_config_set_devcfg_type(&self->hw_config, SYS_DEVCFG_FPGA_MGR);
    sys_hw_config_set_slot_type(&self->hw_config, SYS_SLOT_MASTER);

    // Open reconfiguration device
    retval = devcfg_init(&self->devcfg, &self->hw_config);
    if (retval) {
        ERROR_PRINT("fred_sys: unable to open devcfg device\n");
        goto devcfg_init_error;
    }

    // Initialize scheduler
    retval = sched_fred_init(&self->scheduler, SCHED_FRED_NORMAL, self->devcfg);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing scheduler\n");
        goto sched_init_error;
    }

    // Link scheduler to devcfg
    devcfg_attach_scheduler(self->devcfg, self->scheduler);

    // Initialize base system
    retval = init_base_sys_(self, arch_file, hw_tasks_file);
    if (retval)
        goto base_sys_init_error;

    // Create sw-task listener
    retval = sw_tasks_listener_init(&sw_tasks_listener, self->layout,
                                    self->reactor, self->scheduler, self->buffctl);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing sw-task listener\n");
        goto sw_tasks_listener_init_error;
    }

    // Open file descriptor based signal handler
    retval = signals_recv_init(&signals_receiver);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing fd for receiving signals\n");
        goto signals_recv_init_error;
    }

    // Register file-based signal handler
    retval = reactor_add_event_handler(self->reactor, signals_receiver,
                                        REACT_NORMAL_HANDLER, REACT_OWNED);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering signals handler\n");
        goto handlers_reg_error;
    }

    // Register reconfiguration device event handler
    retval = reactor_add_event_handler(self->reactor, devcfg_get_event_handler(self->devcfg),
                                        REACT_PRI_HANDLER, REACT_NOT_OWNED);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering reconfiguration device handler\n");
        goto handlers_reg_error;
    }

    // Register all slots of all partitions to the reactor
    retval = sys_layout_register_slots(self->layout, self->reactor);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering slots handler\n");
        goto handlers_reg_error;
    }

    // Register sw-task listener
    retval = reactor_add_event_handler(self->reactor, sw_tasks_listener,
                                        REACT_NORMAL_HANDLER, REACT_OWNED);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering sw-task listener handler\n");
        goto handlers_reg_error;
    }

    retval = 0;
    goto out;

handlers_reg_error:
signals_recv_init_error:
    event_handler_free(sw_tasks_listener);
sw_tasks_listener_init_error:
    free_base_sys_(self);
base_sys_init_error:
    // The reactor will automatically free all registered event handlers
    scheduler_free(self->scheduler);
sched_init_error:
    event_handler_free(devcfg_get_event_handler(self->devcfg));
devcfg_init_error:
    retval = -1;
out:
    return retval;
}

//---------------------------------------------------------------------------------------------

int init_rcfg_test_mode_(struct fred_sys *self, const char *arch_file,
                            const char *hw_tasks_file)
{
    struct event_handler *cyclic_client;
    struct event_handler *signals_receiver;
    int retval;

    DBG_PRINT("fred_sys: starting in reconfiguration test mode\n");

    sys_hw_config_set_devcfg_type(&self->hw_config, SYS_DEVCFG_FPGA_MGR);
    sys_hw_config_set_slot_type(&self->hw_config, SYS_SLOT_NULL);

    // Open reconfiguration device
    retval = devcfg_init(&self->devcfg, &self->hw_config);
    if (retval) {
        ERROR_PRINT("fred_sys: unable to open devcfg device\n");
        goto devcfg_init_error;
    }

    // Initialize scheduler
    retval = sched_fred_rand_init(&self->scheduler, self->devcfg);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing scheduler\n");
        goto sched_init_error;
    }

    // Link scheduler to devcfg
    devcfg_attach_scheduler(self->devcfg, self->scheduler);

    // Initialize base system
    retval = init_base_sys_(self, arch_file, hw_tasks_file);
    if (retval)
        goto base_sys_init_error;

    // Create a cyclic client
    retval = cyclic_sw_tasks_client_init(&cyclic_client, self->layout,
                                        self->scheduler, self->buffctl);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing cyclic client\n");
        goto cyclic_client_init_error;
    }

    // Open file descriptor based signal handler
    retval = signals_recv_init(&signals_receiver);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing fd for receiving signals\n");
        goto signals_recv_init_error;
    }

    // Register file-based signal handler
    retval = reactor_add_event_handler(self->reactor, signals_receiver,
                                        REACT_NORMAL_HANDLER, REACT_OWNED);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering signals handler\n");
        goto handlers_reg_error;
    }

    // Register reconfiguration device event handler
    retval = reactor_add_event_handler(self->reactor, devcfg_get_event_handler(self->devcfg),
                                        REACT_PRI_HANDLER, REACT_NOT_OWNED);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering reconfiguration device handler\n");
        goto handlers_reg_error;
    }

    // Register all slots of all partitions to the reactor
    retval = sys_layout_register_slots(self->layout, self->reactor);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering slots handler\n");
        goto handlers_reg_error;
    }

    // Register cyclic client
    retval = reactor_add_event_handler(self->reactor, cyclic_client,
                                        REACT_NORMAL_HANDLER, REACT_OWNED);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering sw-task listener handler\n");
        goto handlers_reg_error;
    }

    retval = 0;
    goto out;

handlers_reg_error:
signals_recv_init_error:
    event_handler_free(cyclic_client);
cyclic_client_init_error:
    free_base_sys_(self);
base_sys_init_error:
    // The reactor will automatically free all registered event handlers
    scheduler_free(self->scheduler);
sched_init_error:
    event_handler_free(devcfg_get_event_handler(self->devcfg));
devcfg_init_error:
    retval = -1;
out:
    return retval;
}

//---------------------------------------------------------------------------------------------

int init_hw_tasks_test_mode_(struct fred_sys *self, const char *arch_file,
                                const char *hw_tasks_file)
{
    struct event_handler *cyclic_client;
    struct event_handler *signals_receiver;
    int retval;

    DBG_PRINT("fred_sys: starting in hw-tasks test mode\n");

    sys_hw_config_set_devcfg_type(&self->hw_config, SYS_DEVCFG_FPGA_MGR);
    sys_hw_config_set_slot_type(&self->hw_config, SYS_SLOT_MASTER);

    // Open reconfiguration device
    retval = devcfg_init(&self->devcfg, &self->hw_config);
    if (retval) {
        ERROR_PRINT("fred_sys: unable to open devcfg device\n");
        goto devcfg_init_error;
    }

    // Initialize scheduler
    retval = sched_fred_rand_init(&self->scheduler, self->devcfg);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing scheduler\n");
        goto sched_init_error;
    }

    // Link scheduler to devcfg
    devcfg_attach_scheduler(self->devcfg, self->scheduler);

    // Initialize base system
    retval = init_base_sys_(self, arch_file, hw_tasks_file);
    if (retval)
        goto base_sys_init_error;

    // Create a cyclic client
    retval = cyclic_sw_tasks_client_init(&cyclic_client, self->layout,
                                        self->scheduler, self->buffctl);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing cyclic client\n");
        goto cyclic_client_init_error;
    }

    // Open file descriptor based signal handler
    retval = signals_recv_init(&signals_receiver);
    if (retval) {
        ERROR_PRINT("fred_sys: error while initializing fd for receiving signals\n");
        goto signals_recv_init_error;
    }

    // Register file-based signal handler
    retval = reactor_add_event_handler(self->reactor, signals_receiver,
                                        REACT_NORMAL_HANDLER, REACT_OWNED);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering signals handler\n");
        goto handlers_reg_error;
    }

    // Register reconfiguration device event handler
    retval = reactor_add_event_handler(self->reactor, devcfg_get_event_handler(self->devcfg),
                                        REACT_PRI_HANDLER, REACT_NOT_OWNED);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering reconfiguration device handler\n");
        goto handlers_reg_error;
    }

    // Register all slots of all partitions to the reactor
    retval = sys_layout_register_slots(self->layout, self->reactor);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering slots handler\n");
        goto handlers_reg_error;
    }

    // Register cyclic client
    retval = reactor_add_event_handler(self->reactor, cyclic_client,
                                        REACT_NORMAL_HANDLER, REACT_OWNED);
    if (retval) {
        ERROR_PRINT("fred_sys: error while registering sw-task listener handler\n");
        goto handlers_reg_error;
    }

    retval = 0;
    goto out;

handlers_reg_error:
signals_recv_init_error:
    event_handler_free(cyclic_client);
cyclic_client_init_error:
    free_base_sys_(self);
base_sys_init_error:
    // The reactor will automatically free all registered event handlers
    scheduler_free(self->scheduler);
sched_init_error:
    event_handler_free(devcfg_get_event_handler(self->devcfg));
devcfg_init_error:
    retval = -1;
out:
    return retval;
}

//---------------------------------------------------------------------------------------------

int fred_sys_init(struct fred_sys **self, const char *arch_file,
                  const char *hw_tasks_file, enum fred_sys_mode mode)
{
    int retval;

    *self = calloc(1, sizeof(**self));
    if (!(*self))
        return -1;

    srand(time(NULL));

    DBG_PRINT(fred_logo);

    switch (mode) {
        case FRED_SYS_RCFG_TEST_MODE:
            retval = init_rcfg_test_mode_(*self, arch_file, hw_tasks_file);
            break;
        case FRED_SYS_HW_TASKS_TEST_MODE:
            retval = init_hw_tasks_test_mode_(*self, arch_file, hw_tasks_file);
            break;
        case FRED_SYS_NORMAL_MODE:
        default:
            retval = init_normal_mode_(*self, arch_file, hw_tasks_file);
            break;
    }

    if (retval) {
        free(*self);
        return -1;
    }

    return 0;
}

void fred_sys_free(struct fred_sys *self)
{
    if (!self)
        return;

    DBG_PRINT("fred_sys: shutting down\n");

    // Will release all registered clients using the
    // free method of the handler
    if (self->reactor)
        reactor_free(self->reactor);

    if (self->scheduler)
        scheduler_free(self->scheduler);

    if (self->layout)
        sys_layout_free(self->layout);

    if (self->devcfg)
        event_handler_free(devcfg_get_event_handler(self->devcfg));

    if (self->buffctl)
        buffctl_close(self->buffctl);

    logger_free();

    free(self);
}

void fred_sys_run(struct fred_sys *self)
{
    assert(self);

    // Start event loop
    reactor_event_loop(self->reactor);
}
