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

#ifndef XDEVCFG_MOD_SHARED_H
#define XDEVCFG_MOD_SHARED_H

#ifdef __KERNEL__
/* Kernel */
#include <linux/types.h>
#include <linux/ioctl.h>
#else
/* User */
#include <stdint.h>
#include <sys/ioctl.h>
typedef struct phy_bitstream phy_bitstream;
#endif

/*******************************************************************************/

/*
 * NOTE: uint32_t and uintptr_t are defined
 * in <stdint.h> for userspace
 * and <linux/types.h> for kernelspace (v >? 4.X)
*/

struct phy_bitstream {
	uintptr_t phy_addr;
	size_t length;
};

/*******************************************************************************/

/* Magic number for IOCTL commands */
#define XDEVCFG_MOD_MAGIC 	0x7e
#define PHY_BIT_TRANSFER	_IOW(XDEVCFG_MOD_MAGIC, 1, struct phy_bitstream)

/*******************************************************************************/

#endif /* XDEVCFG_MOD_SHARED_H */
