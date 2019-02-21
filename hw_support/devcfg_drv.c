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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <unistd.h>
#include <inttypes.h>
#include <sys/ioctl.h>

#include <string.h>

#include "../utils/dbg_print.h"

static const char is_part_path[] = "/sys/devices/soc0/amba/f8007000.devcfg/is_partial_bitstream";
static const char prog_done_path[] = "/sys/devices/soc0/amba/f8007000.devcfg/prog_done";
static const char devcfg_path[] = "/dev/xdevcfg_mod";

struct devcfg_drv_ {
	int xdev_fd;
};

int devcfg_drv_init(devcfg_drv **devcfg)
{
	int fd;
	int retval;

	*devcfg = calloc(1, sizeof(**devcfg));
	if (!(*devcfg))
		return -1;

	// Set devcfg sysfs attribute for partial bistreams
	fd = open(is_part_path, O_RDWR);
	if (fd < 0) {
		free(*devcfg);
		ERROR_PRINT("fred_devcfg: failed to open sysfs partial bistream attribute\n");
		return -1;
	}

	retval = write(fd, "1", 2);
	if (retval < 0) {
		ERROR_PRINT("fred_devcfg: error while setting sysfs attribute for partial bistreams\n");
		return -1;
	}

	close(fd);

	// Open xdevcfg device file
	fd = open(devcfg_path, O_RDWR);
	if (fd < 0) {
		free(*devcfg);
		ERROR_PRINT("fred_devcfg: could not open xdevcfg device file\n");
		return -1;
	}

	(*devcfg)->xdev_fd = fd;

	return 0;
}

void devcfg_drv_free(devcfg_drv *devcfg)
{
	if (!devcfg)
		return;

	close(devcfg->xdev_fd);

	free(devcfg);
}

int devcfg_drv_get_fd(const devcfg_drv *devcfg)
{
	assert(devcfg);

	return devcfg->xdev_fd;
}

int devcfg_drv_start_prog(const devcfg_drv *devcfg,
							const struct phy_bitstream *phy_bit)
{
	int retval;

	assert(devcfg);
	assert(phy_bit);

	retval = ioctl(devcfg->xdev_fd, PHY_BIT_TRANSFER, phy_bit);
	if (retval < 0) {
		ERROR_PRINT("fred_devcfg: phy bitstream transfer fail\n");
		return retval;
	}

	return retval;
}

uint32_t devcfg_drv_clear_evt(const devcfg_drv *devcfg)
{
	uint32_t rcfg_us;
	size_t retval;

	assert(devcfg);

	retval = read(devcfg->xdev_fd, &rcfg_us, sizeof(rcfg_us));
	if (retval != sizeof(rcfg_us)) {
		ERROR_PRINT("fred_devcfg: read for clear event error\n");
		return 0;
	}

	return rcfg_us;
}

int devcfg_drv_write_legacy(const devcfg_drv *devcfg, void *bit, size_t bit_len)
{
	int retval;

	assert(devcfg);

	// Initiate devcfg DMA transfer
	retval = write(devcfg->xdev_fd, bit, bit_len);
	if (retval < 0) {
		ERROR_PRINT("fred_devcfg: error write bitstream legacy\n");
		return -1;
	}

	return 0;
}

