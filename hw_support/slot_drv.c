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

#include <assert.h>
#include <stdio.h>

#include "slot_drv.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

#define SLOT_CTRL_BUS_DEPTH_ARGS		8

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
	volatile uint8_t RESERVED[16];
	volatile uint32_t DATA[SLOT_CTRL_BUS_DEPTH_ARGS];
} __attribute__((packed));
#endif

//---------------------------------------------------------------------------------------------

void slot_drv_enable_irq(uio_dev_ft *uio_dev)
{
	struct slot_regs *regs;

	assert(uio_dev);

	regs = (struct slot_regs *)uio_get_base_addr(uio_dev);

	// Enable interrupt
	regs->IER |= 1U;

	// Enable global interrupt
	regs->GIE = 1U;
}

void slot_drv_clear_int(uio_dev_ft *uio_dev)
{
	struct slot_regs *regs;

	assert(uio_dev);

	regs = (struct slot_regs *)uio_get_base_addr(uio_dev);

	// Clear the local interrupt
	regs->ISR = 1U;
}

uint32_t slot_drv_get_id(const uio_dev_ft *uio_dev)
{
	struct slot_regs *regs;

	assert(uio_dev);

	regs = (struct slot_regs *)uio_get_base_addr(uio_dev);

	return (uint32_t)regs->ID_DATA;
}

int slot_drv_start_compute(uio_dev_ft *uio_dev, const args_t *args, int args_size)
{
	struct slot_regs *regs;

	assert(uio_dev);
	assert(args_size <= SLOT_CTRL_BUS_DEPTH_ARGS);

	regs = (struct slot_regs *)uio_get_base_addr(uio_dev);

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

void slot_drv_wait_for_compl(const uio_dev_ft *uio_dev)
{
	struct slot_regs *regs;

	assert(uio_dev);

	regs = (struct slot_regs *)uio_get_base_addr(uio_dev);

	do {} while (regs->AP_CTRL & 1U);
}
