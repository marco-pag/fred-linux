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

#ifndef FRED_BUFFCTL_SHARED_H
#define FRED_BUFFCTL_SHARED_H

/*
 * NOTE: uint32_t and uintptr_t are defined
 * in <stdint.h> for userspace
 * and <linux/types.h> for kernelspace (v >? 4.X)
*/

#ifdef __KERNEL__
/* Kernel */
#include <linux/types.h>
#include <linux/ioctl.h>
#else
/* User */
#include <stdint.h>
#include <stddef.h>
#include <sys/ioctl.h>
#endif

/*******************************************************************************/

#define FB_DEVN_SIZE 64

/*******************************************************************************/

struct fred_buff_if {
    uint32_t id;
    size_t length;
    uintptr_t phy_addr;
    char dev_name[FB_DEVN_SIZE];
};

/*******************************************************************************/

/* Magic number for IOCTL commands */
#define FRED_BUFFCTL_MAGIC 0x7f

#define FRED_BUFFCTL_ALLOC  _IOWR(FRED_BUFFCTL_MAGIC, 1 , struct fred_buff_if)
#define FRED_BUFFCTL_FREE   _IOW(FRED_BUFFCTL_MAGIC, 2 , uint32_t)

/*******************************************************************************/

#ifndef __KERNEL__
static inline
uint32_t fred_buff_if_get_id(const struct fred_buff_if *self)
{
    return self->id;
}

static inline
size_t fred_buff_if_get_lenght(const struct fred_buff_if *self)
{
    return self->length;
}

static inline
uintptr_t fred_buff_if_get_phy_addr(const struct fred_buff_if *self)
{
    return self->phy_addr;
}

static inline
const char *fred_buff_if_get_name(const struct fred_buff_if *self)
{
    return self->dev_name;
}
#endif

/*******************************************************************************/

#endif /* FRED_BUFFCTL_SHARED_H */
