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
#include <errno.h>

#include "devcfg_drv_fpga_mgr.h"
#include "../srv_core/phy_bit.h"
#include "../utils/dbg_print.h"

//---------------------------------------------------------------------------------------------

static const char *phy_bit_rcfg_done_path =
                "/sys/class/fpga_manager/fpga0/phy_bit_rcfg_done";
static const char *phy_bit_rcfg_start_path =
                "/sys/class/fpga_manager/fpga0/phy_bit_rcfg_start";
static const char *phy_bit_addr_path =
                "/sys/class/fpga_manager/fpga0/phy_bit_addr";
static const char *phy_bit_size_path =
                "/sys/class/fpga_manager/fpga0/phy_bit_size";

//---------------------------------------------------------------------------------------------

int devcfg_drv_fpga_mgr_get_fd_(struct devcfg_drv *self)
{
    struct devcfg_drv_fpga_mgr *drv_fpga_mgr;

    assert(self);

    drv_fpga_mgr = (struct devcfg_drv_fpga_mgr *)self;

    return drv_fpga_mgr->phy_bit_rcfg_done_fd;
}

static inline
int devcfg_drv_fpga_mgr_start_rcfg_(struct devcfg_drv *self, const struct phy_bit *phy_bit)
{
    struct devcfg_drv_fpga_mgr *drv_fpga_mgr;
    int retval;
    char addr_str[24];
    char size_str[24];

    assert(self);
    assert(phy_bit);

    drv_fpga_mgr = (struct devcfg_drv_fpga_mgr *)self;

    snprintf(addr_str, sizeof (addr_str), "%#"PRIxPTR"\n", phy_bit_get_addr(phy_bit));
    snprintf(size_str, sizeof (addr_str), "%zu\n", phy_bit_get_size(phy_bit));

    // Write address
    retval = write(drv_fpga_mgr->phy_bit_addr_fd, &addr_str, sizeof (addr_str));
    if (retval <= 0) {
        ERROR_PRINT("fred_devcfg: phy bit addr write fail: %s\n", strerror(errno));
        return -1;
    }

    // Write size
    retval = write(drv_fpga_mgr->phy_bit_size_fd, &size_str, sizeof(size_str));
    if (retval <= 0) {
        ERROR_PRINT("fred_devcfg: phy bit size write fail: %s\n", strerror(errno));
        return -1;
    }

    // Start reconfiguration
    retval = write(drv_fpga_mgr->phy_bit_rcfg_start_fd, "1", 2);
    if (retval <= 0) {
        ERROR_PRINT("fred_devcfg: phy bit rcfg write fail: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static inline
uint64_t devcfg_drv_fpga_mgr_after_rcfg_(struct devcfg_drv *self)
{
    struct devcfg_drv_fpga_mgr *drv_fpga_mgr;
    int retval;
    int64_t rcfg_us;
    char rcfg_us_str[24];

    assert(self);

    drv_fpga_mgr = (struct devcfg_drv_fpga_mgr *)self;

    lseek(drv_fpga_mgr->phy_bit_rcfg_done_fd, 0, SEEK_SET);

    retval = read(drv_fpga_mgr->phy_bit_rcfg_done_fd, &rcfg_us_str, sizeof(rcfg_us_str));
    if (retval <= 0) {
        ERROR_PRINT("fred_devcfg: read for clear event fail: %s\n", strerror(errno));
        return -1;
    }

    sscanf(rcfg_us_str, "%"PRId64, &rcfg_us);

    return (uint64_t)rcfg_us;
}

static inline
void devcfg_drv_fpga_mgr_free_(struct devcfg_drv *self)
{
    struct devcfg_drv_fpga_mgr *drv_fpga_mgr;

    if (!self)
        return;

    drv_fpga_mgr = (struct devcfg_drv_fpga_mgr *)self;

    close(drv_fpga_mgr->phy_bit_size_fd);
    close(drv_fpga_mgr->phy_bit_addr_fd);
    close(drv_fpga_mgr->phy_bit_rcfg_done_fd);
    close(drv_fpga_mgr->phy_bit_rcfg_start_fd);

    free(drv_fpga_mgr);
}

//---------------------------------------------------------------------------------------------

int devcfg_drv_fpga_mgr_init(struct devcfg_drv **self)
{
    struct devcfg_drv_fpga_mgr *drv_fpga_mgr;
    int retval;
    int64_t rcfg_us;

    assert(self);

    *self = NULL;

    drv_fpga_mgr = calloc(1, sizeof(*drv_fpga_mgr));
    if (!drv_fpga_mgr)
        return -1;

    // Open sysfs rcfg done pollable attribute
    drv_fpga_mgr->phy_bit_rcfg_done_fd = open(phy_bit_rcfg_done_path, O_RDONLY);
    if (drv_fpga_mgr->phy_bit_rcfg_done_fd < 0) {
        free(drv_fpga_mgr);
        ERROR_PRINT("fred_devcfg: failed to open sysfs phy_bit_rcfg_done "
                    "attribute: %s\n", strerror(errno));
        return -1;
    }

    // Open sysfs rcfg start attribute
    drv_fpga_mgr->phy_bit_rcfg_start_fd = open(phy_bit_rcfg_start_path, O_WRONLY);
    if (drv_fpga_mgr->phy_bit_rcfg_start_fd < 0) {
        close(drv_fpga_mgr->phy_bit_rcfg_done_fd);
        free(drv_fpga_mgr);
        ERROR_PRINT("fred_devcfg: failed to open sysfs phy_bit_rcfg_start "
                    "attribute: %s\n", strerror(errno));
        return -1;
    }

    // Open sysfs phy bit address write only attribute
    drv_fpga_mgr->phy_bit_addr_fd = open(phy_bit_addr_path, O_WRONLY);
    if (drv_fpga_mgr->phy_bit_addr_fd < 0) {
        close(drv_fpga_mgr->phy_bit_rcfg_done_fd);
        close(drv_fpga_mgr->phy_bit_rcfg_start_fd);
        free(drv_fpga_mgr);
        ERROR_PRINT("fred_devcfg: failed to open sysfs phy_bit_addr "
                    "attribute: %s\n", strerror(errno));
        return -1;
    }

    // Open sysfs phy bit size write only attribute
    drv_fpga_mgr->phy_bit_size_fd = open(phy_bit_size_path, O_WRONLY);
    if (drv_fpga_mgr->phy_bit_size_fd < 0) {
        close(drv_fpga_mgr->phy_bit_addr_fd);
        close(drv_fpga_mgr->phy_bit_rcfg_done_fd);
        close(drv_fpga_mgr->phy_bit_rcfg_start_fd);
        free(drv_fpga_mgr);
        ERROR_PRINT("fred_devcfg: failed to open sysfs phy_bit_size "
                    "attribute: %s\n", strerror(errno));
        return -1;
    }

    // Perform an initial dummy read on the rcfg attribute
    retval = read(drv_fpga_mgr->phy_bit_rcfg_done_fd, &rcfg_us, sizeof(rcfg_us));
    if (retval <= 0) {
        close(drv_fpga_mgr->phy_bit_size_fd);
        close(drv_fpga_mgr->phy_bit_addr_fd);
        close(drv_fpga_mgr->phy_bit_rcfg_done_fd);
        close(drv_fpga_mgr->phy_bit_rcfg_start_fd);
        free(drv_fpga_mgr);
        ERROR_PRINT("fred_devcfg: error while performing inital dummy "
                    "read on sysfs rcfg attribute: %s\n", strerror(errno));
        return -1;
    }

    // Devcfg interface
    drv_fpga_mgr->devcfg_drv.get_fd = devcfg_drv_fpga_mgr_get_fd_;
    drv_fpga_mgr->devcfg_drv.start_rcfg = devcfg_drv_fpga_mgr_start_rcfg_;
    drv_fpga_mgr->devcfg_drv.after_rcfg = devcfg_drv_fpga_mgr_after_rcfg_;
    drv_fpga_mgr->devcfg_drv.free = devcfg_drv_fpga_mgr_free_;

    *self = &drv_fpga_mgr->devcfg_drv;

    return 0;
}

