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

// Registers offsets from Vivado HLS
#define SLOT_CTRL_BUS_ADDR_AP_CTRL   0x00
#define SLOT_CTRL_BUS_ADDR_GIE       0x04
#define SLOT_CTRL_BUS_ADDR_IER       0x08
#define SLOT_CTRL_BUS_ADDR_ISR       0x0c
#define SLOT_CTRL_BUS_ADDR_ID_DATA   0x10
#define SLOT_CTRL_BUS_BITS_ID_DATA   32
#define SLOT_CTRL_BUS_ADDR_ID_CTRL   0x14
#define SLOT_CTRL_BUS_ADDR_ARGS_BASE 0x20
#define SLOT_CTRL_BUS_ADDR_ARGS_HIGH 0x3f
#define SLOT_CTRL_BUS_WIDTH_ARGS     32
#define SLOT_CTRL_BUS_DEPTH_ARGS     8

#define REG_WRITE(BaseAddress, RegOffset, Data) \
    *(volatile unsigned int *)((BaseAddress) + (RegOffset)) = (unsigned int)(Data)

#define REG_READ(BaseAddress, RegOffset) \
    *(volatile unsigned int *)((BaseAddress) + (RegOffset))

void slot_drv_enable_irq(uio_dev_ft *uio_dev)
{
	uint32_t reg;
	uint32_t base_addr;

	assert(uio_dev);

	base_addr = uio_get_base_addr(uio_dev);

	// Enable interrupt
	reg =  REG_READ(base_addr, SLOT_CTRL_BUS_ADDR_IER);
	REG_WRITE(base_addr, SLOT_CTRL_BUS_ADDR_IER, reg | 0x1);

	// Enable global interrupt
	REG_WRITE(base_addr, SLOT_CTRL_BUS_ADDR_GIE, 1);
}

void slot_drv_clear_int(uio_dev_ft *uio_dev)
{
	uint32_t base_addr;

	assert(uio_dev);

	base_addr = uio_get_base_addr(uio_dev);

	// Clear the local interrupt
	REG_WRITE(base_addr, SLOT_CTRL_BUS_ADDR_ISR, 0x1);
}

uint32_t slot_drv_get_id(const uio_dev_ft *uio_dev)
{
	uint32_t base_addr;

	assert(uio_dev);

	base_addr = uio_get_base_addr(uio_dev);

	return REG_READ(base_addr, SLOT_CTRL_BUS_ADDR_ID_DATA);
}

int slot_drv_start_compute(uio_dev_ft *uio_dev, const args_t *args, int args_size)
{
	uint32_t reg;
	uint32_t base_addr;

	assert(uio_dev);
	assert(args_size < SLOT_CTRL_BUS_DEPTH_ARGS);

	base_addr = uio_get_base_addr(uio_dev);

	// Feed arguments to the accelerator trough AXI control bus
	for (int i = 0; i < args_size; ++i) {
		REG_WRITE(	base_addr,
					SLOT_CTRL_BUS_ADDR_ARGS_BASE + i * sizeof(args_t),
					args[i]);
	}

	// Check if the module is ready
	reg = REG_READ(base_addr, SLOT_CTRL_BUS_ADDR_AP_CTRL);
	if (reg & 0x1) {
		DBG_PRINT("slot_drv: error: hw accelerator is not ready!\n");
		return -1;
	}

	// Start the computation
	REG_WRITE(base_addr, SLOT_CTRL_BUS_ADDR_AP_CTRL, reg | 0x01);

	return 0;
}

void slot_drv_wait_for_compl(const uio_dev_ft *uio_dev)
{
	uint32_t reg;
	uint32_t base_addr;

	assert(uio_dev);

	base_addr = uio_get_base_addr(uio_dev);

	do {
		reg = REG_READ(base_addr, SLOT_CTRL_BUS_ADDR_AP_CTRL);
	} while( reg & 0x1 );
}



