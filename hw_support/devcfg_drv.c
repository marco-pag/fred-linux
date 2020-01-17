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

#include "devcfg_drv.h"
#include "../srv_core/phy_bit.h"
#include "../utils/dbg_print.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>

//---------------------------------------------------------------------------------------------

static const char *phy_bit_rcfg_path = "/sys/class/fpga_manager/fpga0/device/phy_bit_rcfg";
static const char *phy_bit_addr_path = "/sys/class/fpga_manager/fpga0/device/phy_bit_addr";
static const char *phy_bit_size_path = "/sys/class/fpga_manager/fpga0/device/phy_bit_size";

//---------------------------------------------------------------------------------------------

struct devcfg_drv_ {
	int phy_bit_rcfg_fd;
	int phy_bit_addr_fd;
	int phy_bit_size_fd;
};

//---------------------------------------------------------------------------------------------

int devcfg_drv_init(devcfg_drv **devcfg)
{
	int retval;
	int64_t rcfg_us;

	*devcfg = calloc(1, sizeof(**devcfg));
	if (!(*devcfg))
		return -1;

	// Open sysfs rcfg pollable attribute
	(*devcfg)->phy_bit_rcfg_fd = open(phy_bit_rcfg_path, O_RDWR);
	if ((*devcfg)->phy_bit_rcfg_fd < 0) {
		free(*devcfg);
		ERROR_PRINT("fred_devcfg: failed to open sysfs phy_bit_rcfg attribute\n");
		return -1;
	}

	// Open sysfs phy bit address write only attribute
	(*devcfg)->phy_bit_addr_fd = open(phy_bit_addr_path, O_WRONLY);
	if ((*devcfg)->phy_bit_addr_fd < 0) {
		close((*devcfg)->phy_bit_rcfg_fd);
		free(*devcfg);
		ERROR_PRINT("fred_devcfg: failed to open sysfs phy_bit_addr attribute\n");
		return -1;
	}

	// Open sysfs phy bit size write only attribute
	(*devcfg)->phy_bit_size_fd = open(phy_bit_size_path, O_WRONLY);
	if ((*devcfg)->phy_bit_size_fd < 0) {
		close((*devcfg)->phy_bit_addr_fd);
		close((*devcfg)->phy_bit_rcfg_fd);
		free(*devcfg);
		ERROR_PRINT("fred_devcfg: failed to open sysfs phy_bit_size attribute\n");
		return -1;
	}

	// Perform an initial dummy read on the rcfg attribute
	retval = read((*devcfg)->phy_bit_rcfg_fd, &rcfg_us, sizeof(rcfg_us));
	if (retval < 0) {
		close((*devcfg)->phy_bit_size_fd);
		close((*devcfg)->phy_bit_addr_fd);
		close((*devcfg)->phy_bit_rcfg_fd);
		free(*devcfg);
		ERROR_PRINT("fred_devcfg: error while performing inital dummy"
					"read on sysfs rcfg attribute\n");
		return -1;
	}


	return 0;
}

void devcfg_drv_free(devcfg_drv *devcfg)
{
	if (!devcfg)
		return;

	close(devcfg->phy_bit_size_fd);
	close(devcfg->phy_bit_addr_fd);
	close(devcfg->phy_bit_rcfg_fd);

	free(devcfg);
}

int devcfg_drv_get_fd(const devcfg_drv *devcfg)
{
	assert(devcfg);

	return devcfg->phy_bit_rcfg_fd;
}

int devcfg_drv_start_prog(const devcfg_drv *devcfg, const struct phy_bit *phy_bit)
{
	int retval;
	uintptr_t addr;
	size_t size;

	assert(devcfg);
	assert(phy_bit);

	addr = phy_bit_get_addr(phy_bit);
	size = phy_bit_get_size(phy_bit);

	// Write address
	retval = write(devcfg->phy_bit_addr_fd, &addr, sizeof(addr));
	if (retval < 0) {
		ERROR_PRINT("fred_devcfg: phy bit addr write fail\n");
		return retval;
	}

	// Write size
	retval = write(devcfg->phy_bit_size_fd, &size, sizeof(size));
	if (retval < 0) {
		ERROR_PRINT("fred_devcfg: phy bit size write fail\n");
		return retval;
	}

	// Start reconfiguration
	retval = write(devcfg->phy_bit_rcfg_fd, "1", 1);
	if (retval < 0) {
		ERROR_PRINT("fred_devcfg: phy bit rcfg write fail\n");
		return retval;
	}

	return 0;
}

int64_t devcfg_drv_clear_evt(const devcfg_drv *devcfg)
{
	int64_t rcfg_us;
	int retval;

	assert(devcfg);

	retval = read(devcfg->phy_bit_rcfg_fd, &rcfg_us, sizeof(rcfg_us));
	if (retval < 0) {
		ERROR_PRINT("fred_devcfg: read for clear event error\n");
		return -1;
	}

	return rcfg_us;
}

