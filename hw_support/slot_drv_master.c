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

#include "slot_drv_master.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

#define SLOT_CTRL_BUS_DEPTH_ARGS        8

// 64-bit HW-tasks
#ifdef HW_TASKS_A64
struct slot_regs {
    volatile uint32_t AP_CTRL;
    volatile uint32_t GIE;
    volatile uint32_t IER;
    volatile uint32_t ISR;
    volatile uint32_t ID_DATA;
    volatile uint8_t RESERVED[44];
    volatile uint64_t DATA[SLOT_CTRL_BUS_DEPTH_ARGS];
} __attribute__((packed));
// 32-bit HW-tasks
#else
struct slot_regs {
    volatile uint32_t AP_CTRL;
    volatile uint32_t GIE;
    volatile uint32_t IER;
    volatile uint32_t ISR;
    volatile uint32_t ID_DATA;
    volatile uint8_t RESERVED[12];
    volatile uint32_t DATA[SLOT_CTRL_BUS_DEPTH_ARGS];
} __attribute__((packed));
#endif

//---------------------------------------------------------------------------------------------

static
uint32_t slot_drv_master_get_id_(const struct slot_drv *self)
{
    struct slot_drv_master *master_drv;
    struct slot_regs *regs;

    assert(self);

    master_drv = (struct slot_drv_master *)self;

    regs = (struct slot_regs *)uio_get_base_addr(master_drv->uio_dev);

    return (uint32_t)regs->ID_DATA;
}

static
int slot_drv_master_get_fd_(const struct slot_drv *self)
{
    struct slot_drv_master *master_drv;

    assert(self);

    master_drv = (struct slot_drv_master *)self;

    return uio_get_fd(master_drv->uio_dev);
}

static
void slot_drv_master_before_rcfg_(struct slot_drv *self)
{
    assert(self);

    // Empty, no actions required
}

static
void slot_drv_master_after_rcfg_(struct slot_drv *self)
{
    struct slot_drv_master *master_drv;
    struct slot_regs *regs;

    assert(self);

    master_drv = (struct slot_drv_master *)self;

    regs = (struct slot_regs *)uio_get_base_addr(master_drv->uio_dev);

    // Enable interrupt
    regs->IER |= 1U;

    // Enable global interrupt
    regs->GIE = 1U;
}

int slot_drv_master_start_compute_(struct slot_drv *self, const uintptr_t *args,
                                    int args_size)
{
    struct slot_drv_master *master_drv;
    struct slot_regs *regs;

    assert(self);
    assert(args_size <= SLOT_CTRL_BUS_DEPTH_ARGS);

    master_drv = (struct slot_drv_master *)self;

    regs = (struct slot_regs *)uio_get_base_addr(master_drv->uio_dev);

    // Feed arguments to the accelerator trough AXI control bus
    for (int i = 0; i < args_size; ++i)
        regs->DATA[i] = args[i];

    // Check if the module is ready
    if (regs->AP_CTRL & 1U) {
        ERROR_PRINT("slot_drv: error: hw accelerator is not ready!\n");
        return -1;
    }

    // Start the computation
    regs->AP_CTRL |= 1U;

    return 0;
}

void slot_drv_master_after_compute_(struct slot_drv *self)
{
    struct slot_drv_master *master_drv;
    struct slot_regs *regs;

    assert(self);

    master_drv = (struct slot_drv_master *)self;

    regs = (struct slot_regs *)uio_get_base_addr(master_drv->uio_dev);

    // Consume the UIO event
    uio_read_for_irq(master_drv->uio_dev);

    // Interrupt received, clear local interrupt device register
    regs->ISR = 1U;

    // Re-enable interrupt at the controller level
    uio_clear_gic(master_drv->uio_dev);
}

void slot_drv_master_wait_for_compl_(const struct slot_drv *self)
{
    struct slot_drv_master *master_drv;
    struct slot_regs *regs;

    assert(self);

    master_drv = (struct slot_drv_master *)self;

    regs = (struct slot_regs *)uio_get_base_addr(master_drv->uio_dev);

    do {} while (regs->AP_CTRL & 1U);
}

void slot_drv_master_free_(struct slot_drv *self)
{
    struct slot_drv_master *master_drv;

    if (!self)
        return;

    master_drv = (struct slot_drv_master *)self;

    uio_dev_free(master_drv->uio_dev);

    free(master_drv);
}

//---------------------------------------------------------------------------------------------

int slot_drv_master_init(struct slot_drv **self, const char *dev_name)
{
    struct slot_drv_master *master_drv;
    int retval;

    assert(self);

    *self = NULL;

    master_drv = calloc(1, sizeof (*master_drv));
    if (!master_drv)
        return -1;

    // Set properties and methods
    // Slot interface
    master_drv->slot_drv.get_id = slot_drv_master_get_id_;
    master_drv->slot_drv.get_fd = slot_drv_master_get_fd_;
    master_drv->slot_drv.before_rcfg = slot_drv_master_before_rcfg_;
    master_drv->slot_drv.after_rcfg = slot_drv_master_after_rcfg_;
    master_drv->slot_drv.start_compute = slot_drv_master_start_compute_;
    master_drv->slot_drv.after_compute = slot_drv_master_after_compute_;
    master_drv->slot_drv.wait_for_compl = slot_drv_master_wait_for_compl_;
    master_drv->slot_drv.free = slot_drv_master_free_;

    // Initialize UIO component
    retval = uio_dev_init(&master_drv->uio_dev, dev_name);
    if (retval) {
        free(master_drv);
        return -1;
    }

    *self = &master_drv->slot_drv;

    return 0;
}


